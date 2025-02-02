
#include "Asset/Loader/ModelLoader.h"

#include "Graphics/Vertex.h"

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include <spdlog/spdlog.h>

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
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath.string());

		if (!warn.empty())
		{
			SPDLOG_WARN("TING-GLTF Warning: {}", warn);
		}

		if (!err.empty())
		{
			SPDLOG_ERROR("TING-GLTF Error: {}", err);
		}

		if (!ret)
		{
			SPDLOG_ERROR("Failed to parse file: {}", filepath.string());
			return nullptr;
		}

		Ref<Model> engineModel = ConvertModel(model, filepath);

		if (engineModel)
		{
			engineModel->SetFilePath(filepath);
		}

		return engineModel;
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

	Ref<Model> ModelLoader::ConvertModel(const tinygltf::Model& gltfModel, const fs::path& filepath)
	{
		Ref<Model> model = MakeRef<Model>();

		if (gltfModel.scenes.empty()) {
			throw std::runtime_error("GLTF file has no scenes.");
		}

		const tinygltf::Scene& gltfScene = gltfModel.scenes[gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0];

		// Build root node
		ModelNode root;
		for (int nodeIndex : gltfScene.nodes) {
			const tinygltf::Node& gltfNode = gltfModel.nodes[nodeIndex];
			root.AddChild(ConvertNode(gltfModel, gltfNode, filepath));
		}

		model->SetRootNode(root);

		return model;
	}

	ModelNode ModelLoader::ConvertNode(const tinygltf::Model& gltfModel, const tinygltf::Node& gltfNode, const fs::path& filepath)
	{
		ModelNode node;
		node.SetName(gltfNode.name);

		// Convert meshes
		if (gltfNode.mesh >= 0) {
			const tinygltf::Mesh& gltfMesh = gltfModel.meshes[gltfNode.mesh];
			auto meshes = ConvertMesh(gltfModel, gltfMesh, filepath);
			for(auto mesh : meshes)
				node.AddMesh(mesh);
		}

		// Convert children
		for (int childIndex : gltfNode.children) {
			const tinygltf::Node& childNode = gltfModel.nodes[childIndex];
			node.AddChild(ConvertNode(gltfModel, childNode, filepath));
		}

		return node;
	}

	std::vector<Ref<Mesh>> ModelLoader::ConvertMesh(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh, const fs::path& filepath)
	{		
		std::vector<Ref<Mesh>> meshes;

		for (const auto& primitive : gltfMesh.primitives) {

			Buffer vertices;
			Buffer indices;
			// Access vertex positions
			const float* positions = GetBufferAsArray<float>(gltfModel, primitive, "POSITION");
			const float* normals = GetBufferAsArray<float>(gltfModel, primitive, "NORMAL");
			const float* tangents = GetBufferAsArray<float>(gltfModel, primitive, "TANGENT");
			const float* uvs = GetBufferAsArray<float>(gltfModel, primitive, "TEXCOORD_0");
			const float* colors = GetBufferAsArray<float>(gltfModel, primitive, "COLOR_0");

			const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
			
			vertices.Allocate(accessor.count * sizeof(StaticVertex));
			for (int i = 0; i < accessor.count; i++)
			{
				StaticVertex vertex{};
				if (positions) {
					vertex.Position = glm::vec3(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]);
				}
				if (normals) {
					vertex.Normal = glm::vec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
				}
				if (tangents) {
					vertex.Tangent = glm::vec4(tangents[i * 4 + 0], tangents[i * 4 + 1], tangents[i * 4 + 2], tangents[i * 4 + 3]);
				}
				if (uvs) {
					vertex.UV = glm::vec2(uvs[i * 2 + 0], uvs[i * 2 + 1]);
				}
				if (colors) {
					vertex.Color = glm::vec4(colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
				}

				vertices.As<StaticVertex>()[i] = vertex;
			}

			
			const tinygltf::Accessor& indexAccessor = gltfModel.accessors[primitive.indices];

			if (indexAccessor.bufferView < 0)
			{
				vertices.Release();
				continue;
			}

			const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
			const tinygltf::Buffer& indexBuffer = gltfModel.buffers[indexBufferView.buffer];

			IndexBufferType indextype = IndexBufferType::BufferSize_16Bit;
			if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				const uint16_t* indiceArray = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
				indices.Allocate(indexAccessor.count * sizeof(uint16_t));
				memcpy(indices.GetData(), indiceArray, indexAccessor.count * sizeof(uint16_t));
				indextype = IndexBufferType::BufferSize_16Bit;
			}
			else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				const uint32_t* indiceArray = reinterpret_cast<const uint32_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
				indices.Allocate(indexAccessor.count * sizeof(uint32_t));
				memcpy(indices.GetData(), indiceArray, indexAccessor.count * sizeof(uint32_t));
				indextype = IndexBufferType::BufferSize_32Bit;
			}
			else {
				SPDLOG_ERROR("Unsuppoerted index type");
				vertices.Release();
				continue;
			}
			
			if (vertices.GetSize() == 0)
			{
				continue;
			}
			if (indices.GetSize() == 0)
			{
				continue;
			}

			Ref<Material> material = LoadMaterial(gltfModel, gltfModel.materials[primitive.material], filepath);
			mAssetManager->InsertAsset(material);

			MeshDescription meshDescription{};
			meshDescription.Name = gltfMesh.name.empty() ? "Mesh" : gltfMesh.name;
			meshDescription.Indices = indices;
			meshDescription.IndexBufferType = indextype;
			meshDescription.Vertices = vertices;
			meshDescription.VertexSize = sizeof(StaticVertex);
			meshDescription.DefaultMaterialHandle = material->Handle();

			Ref<Mesh> mesh = mGraphicsContext->CreateMesh(meshDescription);
			mAssetManager->InsertAsset(mesh);

			vertices.Release();
			indices.Release();

			meshes.push_back(mesh);
		}

		return meshes;
	}

	Ref<Material> ModelLoader::LoadMaterial(const tinygltf::Model& gltfModel, const tinygltf::Material& gltfMaterial, const fs::path& filepath)
	{
		Ref<Material> material = MakeRef<Material>();

		material->SetName(gltfMaterial.name);

		if (gltfMaterial.alphaMode == "BLEND")
		{
			material->Transparent = true;
			material->Transparency = gltfMaterial.alphaCutoff;
		}

		for(int i = 0; i < 4; i++)
			material->AlbedoColor[i] = gltfMaterial.pbrMetallicRoughness.baseColorFactor[i];

		if (auto texture = LoadTexture(gltfModel, gltfMaterial.pbrMetallicRoughness.baseColorTexture, filepath))
		{
			material->AlbedoMap = texture->Handle();
		}
		if (auto texture = LoadTexture(gltfModel, gltfMaterial.normalTexture, filepath))
		{
			material->NormalMap = texture->Handle();
		}
		if (auto texture = LoadTexture(gltfModel, gltfMaterial.occlusionTexture, filepath))
		{
			material->AOMap = texture->Handle();
			material->AOFactor = 1.f;
		}		
		if (auto texture = LoadTexture(gltfModel, gltfMaterial.emissiveTexture, filepath))
		{
			material->EmissiveMap = texture->Handle();
		}

		std::pair<Ref<Texture2D>, Ref<Texture2D>> metallicRoughnessMaps = LoadMetallicRoughnessTextures(gltfModel, gltfMaterial, filepath);
		
		// Roughness
		if (metallicRoughnessMaps.first)
		{
			material->MetalnessMap = metallicRoughnessMaps.first->Handle();
			material->MetalnessFactor = gltfMaterial.pbrMetallicRoughness.metallicFactor;
		}

		// Metalness
		if (metallicRoughnessMaps.second)
		{
			material->RoughnessMap = metallicRoughnessMaps.second->Handle();
			material->RoughnessFactor = gltfMaterial.pbrMetallicRoughness.roughnessFactor;
		}


		return material;
	}
	
	Ref<Texture2D> ModelLoader::LoadTexture(const tinygltf::Model& gltfModel, const tinygltf::TextureInfo& textureInfo, const fs::path& filepath)
	{
		if (textureInfo.index < 0)
			return nullptr;

		const tinygltf::Texture& gltfTexture = gltfModel.textures[textureInfo.index];
		const tinygltf::Image& image = gltfModel.images[gltfTexture.source];
		fs::path texturePath = filepath.parent_path() / image.uri;

		auto texture = mAssetManager->LoadAsset<Texture2D>(texturePath);

		return texture;
	}

	Ref<Texture2D> ModelLoader::LoadTexture(const tinygltf::Model& gltfModel, const tinygltf::NormalTextureInfo& textureInfo, const fs::path& filepath)
	{
		if (textureInfo.index < 0)
			return nullptr;

		const tinygltf::Texture& gltfTexture = gltfModel.textures[textureInfo.index];
		const tinygltf::Image& image = gltfModel.images[gltfTexture.source];
		fs::path texturePath = filepath.parent_path() / image.uri;

		auto texture = mAssetManager->LoadAsset<Texture2D>(texturePath);

		return texture;
	}

	Ref<Texture2D> ModelLoader::LoadTexture(const tinygltf::Model& gltfModel, const tinygltf::OcclusionTextureInfo& textureInfo, const fs::path& filepath)
	{
		if (textureInfo.index < 0)
			return nullptr;

		const tinygltf::Texture& gltfTexture = gltfModel.textures[textureInfo.index];
		const tinygltf::Image& image = gltfModel.images[gltfTexture.source];
		fs::path texturePath = filepath.parent_path() / image.uri;

		auto texture = mAssetManager->LoadAsset<Texture2D>(texturePath);

		return texture;
	}

	std::pair<Ref<Texture2D>, Ref<Texture2D>> ModelLoader::LoadMetallicRoughnessTextures(const tinygltf::Model& gltfModel, const tinygltf::Material& material, const fs::path& filepath)
	{
		const tinygltf::TextureInfo& textureInfo = material.pbrMetallicRoughness.metallicRoughnessTexture;

		if (textureInfo.index < 0)
			return { nullptr, nullptr };

		const tinygltf::Texture& gltfTexture = gltfModel.textures[textureInfo.index];
		const tinygltf::Image& image = gltfModel.images[gltfTexture.source];
		fs::path texturePath = filepath.parent_path() / image.uri;

		if (image.bits != 8)
		{
			SPDLOG_WARN("GLTF texture has bit depth above 8 bits per channels");
			return { nullptr, nullptr };
		}

		int width = 0, height = 0, channels = 0;
		uint8_t* data = stbi_load(texturePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (data == nullptr || width != image.width || height != image.height)
		{
			SPDLOG_WARN("Failed to load texture: {}, for model: {}", texturePath.filename().string(), filepath.filename().string());
			return { nullptr, nullptr };
		}

		uint8_t* metallicTextureData = new uint8_t[image.width * image.height];
		uint8_t* roughnessTextureData = new uint8_t[image.width * image.height];

		bool emptyRoughness = true;
		bool emptyMetallic = true;

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				metallicTextureData[y * width + x] = data[y * (width * 4) + x * 4 + 2];
				roughnessTextureData[y * width + x] = data[y * (width * 4) + x * 4 + 1];

				if (metallicTextureData[y * width + x] > 0)
					emptyMetallic = false;

				if (roughnessTextureData[y * width + x] > 0)
					emptyRoughness = false;
			}
		}

		Ref<Texture2D> metallicMap = nullptr;
		Ref<Texture2D> roughnessMap = nullptr;

		if (!emptyMetallic)
		{
			metallicMap = MakeRef<Texture2D>(mGraphicsContext, metallicTextureData, width, height, 1, TextureFormat::R8U, TextureFlags::None);
			metallicMap->SetFilePath(texturePath);
			mAssetManager->InsertAsset(metallicMap);
		}

		if (!emptyRoughness)
		{
			roughnessMap = MakeRef<Texture2D>(mGraphicsContext, roughnessTextureData, width, height, 1, TextureFormat::R8U, TextureFlags::None);
			roughnessMap->SetFilePath(texturePath);
			mAssetManager->InsertAsset(roughnessMap);
		}


		return { roughnessMap, metallicMap };
	}
}