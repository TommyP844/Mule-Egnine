
#include "Asset/Loader/ModelLoader.h"
#include "Asset/Loader/AssimpConvert.h"

#include "Graphics/Vertex.h"
#include "ScopedBuffer.h"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <spdlog/spdlog.h>
#include <stb/stb_image.h>

namespace Mule
{
	ModelLoader::ModelLoader(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager)
		:
		mGraphicsContext(context),
		mAssetManager(assetManager)
	{
	}

	Ref<Model> ModelLoader::LoadText(const fs::path& filepath)
	{
		Assimp::Importer importer = Assimp::Importer();

		int flags = aiPostProcessSteps::aiProcess_FlipWindingOrder | aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs;

		const aiScene* scene = importer.ReadFile(filepath.string(), flags);

		if (scene == nullptr)
		{
			SPDLOG_ERROR("Failed to load model: {}", filepath.string());
			return nullptr;
		}

		Ref<Model> model = MakeRef<Model>();
		model->SetFilePath(filepath);
		ModelNode node;
		RecurseNodes(scene, scene->mRootNode, node, filepath);
		model->SetRootNode(node);

		return model;
	}

	void ModelLoader::SaveText(Ref<Model> asset)
	{
	}

	Ref<Model> ModelLoader::LoadBinary(const Buffer& filepath)
	{
		return Ref<Model>();
	}

	void ModelLoader::SaveBinary(Ref<Model> asset)
	{
	}

	void ModelLoader::RecurseNodes(const aiScene* scene, const aiNode* ainode, ModelNode& node, const fs::path& filepath)
	{
		node.SetLocalTransform(toGlm(ainode->mTransformation));
		node.SetName(ainode->mName.C_Str());
		
		for (int i = 0; i < ainode->mNumMeshes; i++)
		{
			unsigned int index = ainode->mMeshes[i];
			Ref<Mesh> mesh = LoadMesh(scene, scene->mMeshes[index], filepath);
			node.AddMesh(mesh);
		}


		for (int i = 0; i < ainode->mNumChildren; i++)
		{
			ModelNode childNode;
			RecurseNodes(scene, ainode->mChildren[i], childNode, filepath);
			node.AddChild(childNode);
		}
	}

	Ref<Mesh> ModelLoader::LoadMesh(const aiScene* scene, const aiMesh* mesh, const fs::path& filepath)
	{
		ScopedBuffer vertices(sizeof(StaticVertex) * mesh->mNumVertices);		
		StaticVertex* verticePtr = vertices.As<StaticVertex>();

		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			StaticVertex v;

			v.Position = toGlm(mesh->mVertices[i]);
			v.Normal = toGlm(mesh->mNormals[i]);
			v.Tangent = toGlm(mesh->mTangents[i]);
			v.UV = toGlm(mesh->mTextureCoords[0][i]);
			v.Color = mesh->HasVertexColors(0) ? toGlm(mesh->mColors[0][i]) : glm::vec4(1.f);

			verticePtr[i] = v;
		}

		IndexBufferType indexType;
		ScopedBuffer indices;
		if (mesh->mNumFaces * 3 > UINT16_MAX)
		{
			indices.Allocate(sizeof(uint32_t) * mesh->mNumFaces * 3);
			uint32_t* indexPtr = indices.As<uint32_t>();
			indexType = IndexBufferType::BufferSize_32Bit;
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
			indexType = IndexBufferType::BufferSize_16Bit;
			uint16_t* indexPtr = indices.As<uint16_t>();
			for (int i = 0; i < mesh->mNumFaces; i++)
			{
				indexPtr[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
				indexPtr[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
				indexPtr[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
			}
		}

		Ref<Material> material = LoadMaterial(scene, scene->mMaterials[mesh->mMaterialIndex], filepath);

		MeshDescription meshDesc{};
		meshDesc.Name = mesh->mName.C_Str();
		meshDesc.Vertices = vertices;
		meshDesc.VertexSize = sizeof(StaticVertex);
		meshDesc.IndexBufferType = indexType;
		meshDesc.Indices = indices;
		meshDesc.DefaultMaterialHandle = material ? material->Handle() : NullAssetHandle;

		Ref<Mesh> muleMesh = mGraphicsContext->CreateMesh(meshDesc);
		mAssetManager->InsertAsset(muleMesh);	

		return muleMesh;
	}
	
	Ref<Material> ModelLoader::LoadMaterial(const aiScene* scene, const aiMaterial* material, const fs::path& filepath)
	{
		Ref<Material> mat = MakeRef<Material>();
		mat->SetName(material->GetName().C_Str());

		// Albedo
		{
			aiString path;
			aiReturn result = material->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
			if (result == aiReturn_SUCCESS)
			{
				if (path.data[0] == '*')
				{
					const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
					Ref<Texture2D> albedoMap = LoadTexture(scene, texture, filepath);
					mat->AlbedoMap = albedoMap ? albedoMap->Handle() : NullAssetHandle;
				}
				else
				{
					fs::path p = filepath.parent_path() / path.C_Str();
					Ref<Texture2D> albedo = mAssetManager->LoadAsset<Texture2D>(p);
					mat->AlbedoMap = albedo ? albedo->Handle() : NullAssetHandle;
				}
			}
			else
			{
				result = material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
				if (result == aiReturn_SUCCESS)
				{
					if (path.data[0] == '*')
					{
						const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
						Ref<Texture2D> albedoMap = LoadTexture(scene, texture, filepath);
						mat->AlbedoMap = albedoMap ? albedoMap->Handle() : NullAssetHandle;
					}
					else
					{
						fs::path p = filepath.parent_path() / path.C_Str();
						Ref<Texture2D> albedo = mAssetManager->LoadAsset<Texture2D>(p);
						mat->AlbedoMap = albedo ? albedo->Handle() : NullAssetHandle;
					}
				}
			}
		}

		// Normals
		{
			aiString path;
			aiReturn result = material->GetTexture(aiTextureType_NORMALS, 0, &path);
			if (result == aiReturn_SUCCESS)
			{
				if (path.data[0] == '*')
				{
					const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
					Ref<Texture2D> normalMap = LoadTexture(scene, texture, filepath);
					mat->NormalMap = normalMap ? normalMap->Handle() : NullAssetHandle;
				}
				else
				{
					fs::path p = filepath.parent_path() / path.C_Str();
					Ref<Texture2D> normalMap = mAssetManager->LoadAsset<Texture2D>(p);
					mat->NormalMap = normalMap ? normalMap->Handle() : NullAssetHandle;
				}
			}
		}

		// metallic / Roughness
		{
			aiString path;
			aiReturn result = material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &path);
			if (result == aiReturn_SUCCESS)
			{
				
				fs::path p = filepath.parent_path() / path.C_Str();
				
				int width, height, channels;
				uint8_t* data = stbi_load(p.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
				if (data != nullptr)
				{
					ScopedBuffer roughness(width * height * 4);
					ScopedBuffer metallic(width * height * 4);

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

					Ref<Texture2D> metallicTex = MakeRef<Texture2D>(mGraphicsContext, metallicPtr, width, height, 1, TextureFormat::RGBA8U);
					if (metallicTex)
					{
						mAssetManager->InsertAsset(metallicTex);
						mat->MetalnessMap = metallicTex->Handle();
					}
					
					Ref<Texture2D> roughnessTex = MakeRef<Texture2D>(mGraphicsContext, roughnessPtr, width, height, 1, TextureFormat::RGBA8U);
					if (roughnessTex)
					{
						mAssetManager->InsertAsset(roughnessTex);
						mat->RoughnessMap = roughnessTex->Handle();
					}
				}
			}
			else
			{
				// Metalness
				{
					aiString path;
					aiReturn result = material->GetTexture(aiTextureType_METALNESS, 0, &path);
					if (result == aiReturn_SUCCESS)
					{
						if (path.data[0] == '*')
						{
							const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
							Ref<Texture2D> metalnessMap = LoadTexture(scene, texture, filepath);
							mat->MetalnessMap = metalnessMap ? metalnessMap->Handle() : NullAssetHandle;
						}
						else
						{
							fs::path p = filepath.parent_path() / path.C_Str();
							Ref<Texture2D> metalnessMap = mAssetManager->LoadAsset<Texture2D>(p);
							mat->MetalnessMap = metalnessMap ? metalnessMap->Handle() : NullAssetHandle;
						}
					}
				}

				// Roughness
				{
					aiString path;
					aiReturn result = material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path);
					if (result == aiReturn_SUCCESS)
					{
						if (path.data[0] == '*')
						{
							const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
							Ref<Texture2D> roughnessMap = LoadTexture(scene, texture, filepath);
							mat->RoughnessMap = roughnessMap ? roughnessMap->Handle() : NullAssetHandle;
						}
						else
						{
							fs::path p = filepath.parent_path() / path.C_Str();
							Ref<Texture2D> roughnessMap = mAssetManager->LoadAsset<Texture2D>(p);
							mat->RoughnessMap = roughnessMap ? roughnessMap->Handle() : NullAssetHandle;
						}
					}
				}
			}
		}

		// Ambient Occlusion
		{
			aiString path;
			aiReturn result = material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &path);
			if (result == aiReturn_SUCCESS)
			{
				if (path.data[0] == '*')
				{
					const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
					Ref<Texture2D> ambientOccMap = LoadTexture(scene, texture, filepath);
					mat->AOMap = ambientOccMap ? ambientOccMap->Handle() : NullAssetHandle;
				}
				else
				{
					fs::path p = filepath.parent_path() / path.C_Str();
					Ref<Texture2D> ambientOccMap = mAssetManager->LoadAsset<Texture2D>(p);
					mat->AOMap = ambientOccMap ? ambientOccMap->Handle() : NullAssetHandle;
				}
			}
			else if (aiReturn_SUCCESS == material->GetTexture(aiTextureType_AMBIENT, 0, &path))
			{
				if (path.data[0] == '*')
				{
					const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
					Ref<Texture2D> ambientOccMap = LoadTexture(scene, texture, filepath);
					mat->AOMap = ambientOccMap ? ambientOccMap->Handle() : NullAssetHandle;
				}
				else
				{
					fs::path p = filepath.parent_path() / path.C_Str();
					Ref<Texture2D> ambientOccMap = mAssetManager->LoadAsset<Texture2D>(p);
					mat->AOMap = ambientOccMap ? ambientOccMap->Handle() : NullAssetHandle;
				}
			}
			else if (aiReturn_SUCCESS == material->GetTexture(aiTextureType_LIGHTMAP, 0, &path))
			{
				if (path.data[0] == '*')
				{
					const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
					Ref<Texture2D> ambientOccMap = LoadTexture(scene, texture, filepath);
					mat->AOMap = ambientOccMap ? ambientOccMap->Handle() : NullAssetHandle;
				}
				else
				{
					fs::path p = filepath.parent_path() / path.C_Str();
					Ref<Texture2D> ambientOccMap = mAssetManager->LoadAsset<Texture2D>(p);
					mat->AOMap = ambientOccMap ? ambientOccMap->Handle() : NullAssetHandle;
				}
			}
		}

		// Emissive
		{
			aiString path;
			aiReturn result = material->GetTexture(aiTextureType_EMISSIVE, 0, &path);
			if (result == aiReturn_SUCCESS)
			{
				if (path.data[0] == '*')
				{
					const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
					Ref<Texture2D> emissiveMap = LoadTexture(scene, texture, filepath);
					mat->EmissiveMap = emissiveMap ? emissiveMap->Handle() : NullAssetHandle;
				}
				else
				{
					fs::path p = filepath.parent_path() / path.C_Str();
					Ref<Texture2D> emissiveMap = mAssetManager->LoadAsset<Texture2D>(p);
					mat->EmissiveMap = emissiveMap ? emissiveMap->Handle() : NullAssetHandle;
				}
			}
		}

		mAssetManager->InsertAsset(mat);

		return mat;
	}
	
	Ref<Texture2D> ModelLoader::LoadTexture(const aiScene* scene, const aiTexture* texture, const fs::path& filepath)
	{
		Ref<Texture2D> tex;

		if (texture->mHeight == 0)
		{
			int width, height, channels;
			void* data = stbi_load_from_memory(&texture->pcData[0].r, texture->mWidth, &width, &height, &channels, STBI_rgb_alpha);
			if (data == nullptr)
				return nullptr;
			
			tex = MakeRef<Texture2D>(mGraphicsContext, data, width, height, 1, TextureFormat::RGBA8U);
		}
		else
		{
			tex = MakeRef<Texture2D>(mGraphicsContext, texture->pcData, texture->mWidth, texture->mHeight, 1, TextureFormat::RGBA8U);
		}

		tex->SetName(texture->mFilename.C_Str());

		return tex;
	}
}