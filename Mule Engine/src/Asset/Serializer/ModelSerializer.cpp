
#include "Asset/Serializer/ModelSerializer.h"
#include "Asset/Serializer/Convert/AssimpConvert.h"
#include "Asset/Serializer/Convert/YamlConvert.h"

#include "Graphics/Vertex.h"
#include "ScopedBuffer.h"
#include "Engine Context/EngineContext.h"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <spdlog/spdlog.h>

#include <stb/stb_image.h>

#include <yaml-cpp/yaml.h>

// STD
#include <fstream>
#include <format>

namespace Mule
{
	ModelSerializer::ModelSerializer(Ref<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	Ref<Model> ModelSerializer::Load(const fs::path& filepath)
	{
		Assimp::Importer importer = Assimp::Importer();

		int flags = aiPostProcessSteps::aiProcess_FlipWindingOrder | aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;

		const aiScene* scene = importer.ReadFile(filepath.string(), flags);

		if (scene == nullptr)
		{
			SPDLOG_ERROR("Failed to load model: {}", filepath.string());
			return nullptr;
		} 

		Ref<Model> model = MakeRef<Model>();
		model->SetFilePath(filepath);
		ModelNode node;
		
		LoadInfo info = LoadInfo(filepath, scene);
		info.Min.x = std::numeric_limits<float>::max();
		info.Min.y = std::numeric_limits<float>::max();
		info.Min.z = std::numeric_limits<float>::max();

		info.Max.x = std::numeric_limits<float>::min();
		info.Max.y = std::numeric_limits<float>::min();
		info.Max.z = std::numeric_limits<float>::min();

		fs::path metaPath = filepath.string() + ".yml";

		LoadSerializationInfo(metaPath, info);

		RecurseNodes(scene->mRootNode, node, info);
		model->SetRootNode(node);
		model->SetMin(info.Min);
		model->SetMax(info.Max);

		BuildSerializationInfo(metaPath, model);

		return model;
	}

	void ModelSerializer::Save(Ref<Model> asset)
	{
	}

	void ModelSerializer::RecurseNodes(const aiNode* ainode, ModelNode& node, LoadInfo& info)
	{
		glm::mat4 nodeTransfrom = toGlm(ainode->mTransformation);
		node.SetLocalTransform(nodeTransfrom);
		node.SetName(ainode->mName.C_Str());
		
		fs::path filepath = info.Filepath;

		for (int i = 0; i < ainode->mNumMeshes; i++)
		{
			unsigned int index = ainode->mMeshes[i];
			Ref<Mesh> mesh = LoadMesh(info.Scene->mMeshes[index], info);
			node.AddMesh(mesh);
		}


		for (int i = 0; i < ainode->mNumChildren; i++)
		{
			ModelNode childNode;
			RecurseNodes(ainode->mChildren[i], childNode, info);
			node.AddChild(childNode);
		}
	}

	Ref<Mesh> ModelSerializer::LoadMesh(const aiMesh* mesh, LoadInfo& info)
	{
		ScopedBuffer vertices(sizeof(StaticVertex) * mesh->mNumVertices);		
		StaticVertex* verticePtr = vertices.As<StaticVertex>();

		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			StaticVertex v;

			v.Position = toGlm(mesh->mVertices[i]);
			v.Normal = toGlm(mesh->mNormals[i]);
			v.Tangent = mesh->HasTangentsAndBitangents() ? toGlm(mesh->mTangents[i]) : glm::vec3(0.f);
			v.UV = toGlm(mesh->mTextureCoords[0][i]);
			v.Color = mesh->HasVertexColors(0) ? toGlm(mesh->mColors[0][i]) : glm::vec4(1.f);

			glm::vec3 pos = v.Position;

			info.Min.x = glm::min(info.Min.x, pos.x);
			info.Min.y = glm::min(info.Min.y, pos.y);
			info.Min.z = glm::min(info.Min.z, pos.z);

			info.Max.x = glm::max(info.Max.x, pos.x);
			info.Max.y = glm::max(info.Max.y, pos.y);
			info.Max.z = glm::max(info.Max.z, pos.z);


			verticePtr[i] = v;
		}

		IndexType indexType;
		ScopedBuffer indices;
		if (mesh->mNumFaces * 3 > UINT16_MAX)
		{
			indices.Allocate(sizeof(uint32_t) * mesh->mNumFaces * 3);
			uint32_t* indexPtr = indices.As<uint32_t>();
			indexType = IndexType::Size_32Bit;
			for (int i = 0; i < mesh->mNumFaces; i++)
			{
				indexPtr[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
				indexPtr[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
				indexPtr[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
			}
		}
		else
		{
			indices.Allocate(sizeof(uint16_t) * mesh->mNumFaces * 3);
			indexType = IndexType::Size_16Bit;
			uint16_t* indexPtr = indices.As<uint16_t>();
			for (int i = 0; i < mesh->mNumFaces; i++)
			{
				indexPtr[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
				indexPtr[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
				indexPtr[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
			}
		}

		Ref<Material> material = LoadMaterial(info.Scene->mMaterials[mesh->mMaterialIndex], info);

		auto assetManager = mServiceManager->Get<AssetManager>();

		std::string meshName = CreateAssetName(mesh->mName.C_Str(), info, AssetType::Mesh);

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, VertexLayout());
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, indexType);

		Ref<Mesh> muleMesh = Mesh::Create(
			meshName,
			vertexBuffer,
			indexBuffer,
			material ? material->Handle() : AssetHandle::Null()
		);

		if (muleMesh)
		{
			auto iter = info.Meshes.find(meshName);
			if (iter != info.Meshes.end())
			{
				muleMesh->SetHandle(iter->second);
			}
		}

		assetManager->Insert(muleMesh);

		return muleMesh;
	}
	
	Ref<Material> ModelSerializer::LoadMaterial(const aiMaterial* material, LoadInfo& info)
	{
		Ref<Material> mat = MakeRef<Material>();
		std::string materialName = CreateAssetName(material->GetName().C_Str(), info, AssetType::Material);
		mat->SetName(materialName);

		auto iter = info.Materials.find(materialName);
		if (iter != info.Materials.end())
		{
			mat->SetHandle(iter->second);
		}

		WeakRef<Texture2D> albedoMap = LoadTexture(material, { aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE }, info);
		WeakRef<Texture2D> normalMap = LoadTexture(material, { aiTextureType_NORMALS }, info);
		WeakRef<Texture2D> AOMap = LoadTexture(material, { aiTextureType_AMBIENT_OCCLUSION, aiTextureType_AMBIENT, aiTextureType_LIGHTMAP }, info);
		WeakRef<Texture2D> emissiveMap = LoadTexture(material, { aiTextureType_EMISSIVE }, info);
		Ref<Texture2D> metallicMap, roughnessMap;

		auto assetManager = mServiceManager->Get<AssetManager>();

		// metallic / Roughness
		{
			aiString path;
			aiReturn result = material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &path);
			if (result == aiReturn_SUCCESS)
			{
				fs::path p = info.Filepath.parent_path() / path.C_Str();
				
				int width, height, channels;
				uint8_t* data = stbi_load(p.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
				if (data != nullptr)
				{
					Buffer roughness(width * height * 4);
					Buffer metallic(width * height * 4);

					uint8_t* roughnessPtr = roughness.As<uint8_t>();
					uint8_t* metallicPtr = metallic.As<uint8_t>();

					for (int i = 0; i < width; i++)
					{
						for (int j = 0; j < height; j++)
						{
							roughnessPtr[j * width * 4 + i * 4 + 0] = data[j * width * 4 + i * 4 + 1];
							roughnessPtr[j * width * 4 + i * 4 + 1] = data[j * width * 4 + i * 4 + 1];
							roughnessPtr[j * width * 4 + i * 4 + 2] = data[j * width * 4 + i * 4 + 1];
							roughnessPtr[j * width * 4 + i * 4 + 3] = data[j * width * 4 + i * 4 + 1];

							metallicPtr[j * width * 4 + i * 4 + 0] = data[j * width * 4 + i * 4 + 2];
							metallicPtr[j * width * 4 + i * 4 + 1] = data[j * width * 4 + i * 4 + 2];
							metallicPtr[j * width * 4 + i * 4 + 2] = data[j * width * 4 + i * 4 + 2];
							metallicPtr[j * width * 4 + i * 4 + 3] = data[j * width * 4 + i * 4 + 2];
						}
					}

					
					metallicMap = Texture2D::Create("", roughness, width, height, TextureFormat::RGBA_8U, TextureFlags::GenerateMips | TextureFlags::TransferDst);
					if (metallicMap)
					{
						assetManager->Insert(metallicMap);
						mat->MetalnessMap = metallicMap->Handle();
						metallicMap->SetName(CreateAssetName("", info, AssetType::Texture));
						auto iter = info.Textures.find(metallicMap->Name());
						if (iter != info.Textures.end())
						{
							assetManager->UpdateHandle(metallicMap->Handle(), iter->second);
						}
					}
					
					roughnessMap = Texture2D::Create("", metallic, width, height, TextureFormat::RGBA_8U, TextureFlags::GenerateMips | TextureFlags::TransferDst);
					if (roughnessMap)
					{
						assetManager->Insert(roughnessMap);
						mat->RoughnessMap = roughnessMap->Handle();
						roughnessMap->SetName(CreateAssetName("", info, AssetType::Texture));
						auto iter = info.Textures.find(roughnessMap->Name());
						if (iter != info.Textures.end())
						{
							assetManager->UpdateHandle(roughnessMap->Handle(), iter->second);
						}
					}

					roughness.Release();
					metallic.Release();
				}
			}
			else
			{
				//metallicMap = LoadTexture(material, { aiTextureType_METALNESS }, info);
				//roughnessMap = LoadTexture(material, { aiTextureType_DIFFUSE_ROUGHNESS }, info);
			}
		}

		mat->AlbedoMap = albedoMap ? albedoMap->Handle() : AssetHandle::Null();
		mat->NormalMap = normalMap ? normalMap->Handle() : AssetHandle::Null();
		mat->RoughnessMap = roughnessMap ? roughnessMap->Handle() : AssetHandle::Null();
		mat->MetalnessMap = metallicMap ? metallicMap->Handle() : AssetHandle::Null();
		mat->AOMap = AOMap ? AOMap->Handle() : AssetHandle::Null();
		mat->EmissiveMap = emissiveMap ? emissiveMap->Handle() : AssetHandle::Null();

		assetManager->Insert(mat);

		return mat;
	}

	WeakRef<Texture2D> ModelSerializer::LoadTexture(const aiMaterial* material, const std::vector<aiTextureType>& textureTypes, LoadInfo& info)
	{
		auto assetManager = mServiceManager->Get<AssetManager>();

		WeakRef<Texture2D> tex2d = nullptr;
		aiString path;
		for (auto textureType : textureTypes)
		{
			aiReturn result = material->GetTexture(textureType, 0, &path);
			if (result == aiReturn_SUCCESS)
			{
				if (path.data[0] == '*')
				{
					const aiTexture* texture = info.Scene->GetEmbeddedTexture(path.C_Str());
					tex2d = LoadTexture(texture);
					if (tex2d)
					{
						std::string name = CreateAssetName(texture->mFilename.C_Str(), info, AssetType::Texture);
						tex2d->SetName(name);
						break;
					}
				}
				else
				{
					fs::path p = info.Filepath.parent_path() / path.C_Str();
					tex2d = assetManager->Load<Texture2D>(p);
					if (tex2d)
						break;
				}
			}
		}

		if (tex2d)
		{
			auto iter = info.Textures.find(tex2d->Name());
			if (iter != info.Textures.end())
			{
				assetManager->UpdateHandle(tex2d->Handle(), iter->second);
			}
		}
		return tex2d;
	}
	
	WeakRef<Texture2D> ModelSerializer::LoadTexture(const aiTexture* texture)
	{
		auto assetManager = mServiceManager->Get<AssetManager>();
		auto graphicsContext = mServiceManager->Get<GraphicsContext>();


		Ref<Texture2D> tex;

		if (texture->mHeight == 0)
		{
			int width, height, channels;
			void* data = stbi_load_from_memory(&texture->pcData[0].r, texture->mWidth, &width, &height, &channels, STBI_rgb_alpha);
			if (data == nullptr)
				return nullptr;

			Buffer buffer(data, width * height * 4);
			
			tex = Texture2D::Create("", buffer, width, height, TextureFormat::RGBA_8U, TextureFlags::GenerateMips | TextureFlags::TransferDst);
		}
		else
		{
			Buffer buffer(texture->pcData, texture->mWidth * texture->mHeight * 4);

			tex = Texture2D::Create("", buffer, texture->mWidth, texture->mHeight, TextureFormat::RGBA_8U, TextureFlags::GenerateMips | TextureFlags::TransferDst);
		}

		assetManager->Insert(tex);

		return tex;
	}

	std::string ModelSerializer::CreateAssetName(std::string name, LoadInfo& info, AssetType assetType)
	{
		if (name.empty())
		{
			name = info.Filepath.filename().replace_extension().string();
			switch (assetType)
			{
			case AssetType::Mesh:
				name += "-Mesh." + std::format("{:03d}", info.MeshCount++);
				break;
			case AssetType::Material:
				name += "-Material." + std::format("{:03d}", info.MaterialCount++);
				break;
			case AssetType::Texture:
				name += "-Texture." + std::format("{:03d}", info.TextureCount++);
				break;
			default:
				SPDLOG_WARN("Invalid asset type, {}", __FUNCTION__);
				name += "-UNKNOWN";
				break;
			}
		}
		return name;
	}
	
	void ModelSerializer::LoadSerializationInfo(const fs::path& metaPath, LoadInfo& info)
	{
		if (!fs::exists(metaPath))
			return;

		YAML::Node root = YAML::LoadFile(metaPath.string());

		for (auto it = root["Textures"].begin(); it != root["Textures"].end(); it++)
		{
			std::string key = it->first.as<std::string>();
			AssetHandle value = it->second.as<AssetHandle>();
			info.Textures[key] = value;
		}

		for (auto it = root["Materials"].begin(); it != root["Materials"].end(); it++)
		{
			std::string key = it->first.as<std::string>();
			AssetHandle value = it->second.as<AssetHandle>();
			info.Materials[key] = value;
		}

		for (auto it = root["Meshes"].begin(); it != root["Meshes"].end(); it++)
		{
			std::string key = it->first.as<std::string>();
			AssetHandle value = it->second.as<AssetHandle>();
			info.Meshes[key] = value;
		}
	}

	void ModelSerializer::BuildSerializationInfo(const fs::path& metaPath, Ref<Model> model)
	{
		if (fs::exists(metaPath))
			return;

		YAML::Node root;
		YAML::Node meshes, materials, textures;

		RecurseModelInfo(model->GetRootNode(), meshes, materials, textures);

		root["Meshes"] = meshes;
		root["Materials"] = materials;
		root["Textures"] = textures;

		YAML::Emitter emitter;
		emitter << root;
		std::ofstream file(metaPath);
		if (!file)
		{
			SPDLOG_ERROR("Failed to create meta file for: {}", metaPath.filename().string());
		}
		else
		{
			file << emitter.c_str();
			file.close();
		}

	}

	void ModelSerializer::RecurseModelInfo(const ModelNode& node, YAML::Node& meshes, YAML::Node& materials, YAML::Node& textures)
	{
		auto assetManager = mServiceManager->Get<AssetManager>();

		for (const auto& mesh : node.GetMeshes())
		{
			meshes[mesh->Name()] = mesh->Handle();

			auto materialHandle = mesh->GetDefaultMaterialHandle();
			auto material = assetManager->Get<Material>(materialHandle);
			if (material)
			{
				materials[material->Name()] = material->Handle();
				auto albedoTexture = assetManager->Get<Texture2D>(material->AlbedoMap);
				if (albedoTexture)
				{
					textures[albedoTexture->Name()] = albedoTexture->Handle();
				}

				auto normalTexture = assetManager->Get<Texture2D>(material->NormalMap);
				if (normalTexture)
				{
					textures[normalTexture->Name()] = normalTexture->Handle();
				}

				auto metallicTexture = assetManager->Get<Texture2D>(material->MetalnessMap);
				if (metallicTexture)
				{
					textures[metallicTexture->Name()] = metallicTexture->Handle();
				}

				auto roughnessTexture = assetManager->Get<Texture2D>(material->RoughnessMap);
				if (roughnessTexture)
				{
					textures[roughnessTexture->Name()] = roughnessTexture->Handle();
				}

				auto aoTexture = assetManager->Get<Texture2D>(material->AOMap);
				if (aoTexture)
				{
					textures[aoTexture->Name()] = aoTexture->Handle();
				}

				auto emissiveTexture = assetManager->Get<Texture2D>(material->EmissiveMap);
				if (emissiveTexture)
				{
					textures[emissiveTexture->Name()] = emissiveTexture->Handle();
				}
			}
		}

		for (const auto& child : node.GetChildren())
		{
			RecurseModelInfo(child, meshes, materials, textures);
		}
	}
}