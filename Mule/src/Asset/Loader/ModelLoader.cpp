
#include "Asset/Loader/ModelLoader.h"

#include "Graphics/Vertex.h"

#define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <spdlog/spdlog.h>

namespace Mule
{
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

		Ref<Model> engineModel = ConvertModel(model);

		if (engineModel)
		{
			engineModel->SetFilePath(filepath);
		}

		return engineModel;
	}

	void ModelLoader::SaveText(Ref<Model> asset)
	{
	}

	Ref<Model> ModelLoader::LoadBinary(const fs::path& filepath)
	{
		return Ref<Model>();
	}

	void ModelLoader::SaveBinary(Ref<Model> asset)
	{
	}

	Ref<Model> ModelLoader::ConvertModel(const tinygltf::Model& gltfModel)
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
			root.AddChild(ConvertNode(gltfModel, gltfNode));
		}

		model->SetRootNode(root);

		return model;
	}

	ModelNode ModelLoader::ConvertNode(const tinygltf::Model& gltfModel, const tinygltf::Node& gltfNode)
	{
		ModelNode node;
		node.SetName(gltfNode.name);

		// Convert meshes
		if (gltfNode.mesh >= 0) {
			const tinygltf::Mesh& gltfMesh = gltfModel.meshes[gltfNode.mesh];
			auto meshes = ConvertMesh(gltfModel, gltfMesh);
			for(auto mesh : meshes)
				node.AddMesh(mesh);
		}

		// Convert children
		for (int childIndex : gltfNode.children) {
			const tinygltf::Node& childNode = gltfModel.nodes[childIndex];
			node.AddChild(ConvertNode(gltfModel, childNode));
		}

		return node;
	}

	std::vector<Ref<Mesh>> ModelLoader::ConvertMesh(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh)
	{		
		std::vector<Ref<Mesh>> meshes;

		for (const auto& primitive : gltfMesh.primitives) {
			
			Buffer vertices;
			Buffer indices;
			// Access vertex positions
			std::vector<glm::vec3> positions = GetBufferAsVector<glm::vec3>(gltfModel, primitive, "POSITION");
			std::vector<glm::vec3> normals = GetBufferAsVector<glm::vec3>(gltfModel, primitive, "NORMAL");
			std::vector<glm::vec3> tangents = GetBufferAsVector<glm::vec3>(gltfModel, primitive, "TANGENT");
			std::vector<glm::vec3> uvs = GetBufferAsVector<glm::vec3>(gltfModel, primitive, "TEXCOORD_0");
			std::vector<glm::vec4> colors = GetBufferAsVector<glm::vec4>(gltfModel, primitive, "COLOR_0");

			// check for error
			
			vertices.Allocate(positions.size() * sizeof(StaticVertex));
			for (int i = 0; i < positions.size(); i++)
			{
				StaticVertex v{};

				v.Position = positions[i];
				v.Normal = normals[i];
				v.Tangent = tangents.empty() ? glm::vec3() : tangents[i];
				v.UV = uvs.empty() ? glm::vec2() : uvs[i];
				v.Color = colors.empty() ? glm::vec4() : colors[i];

				vertices.As<StaticVertex>()[i] = v;
			}

			// Access indices

			
			const tinygltf::Accessor& indexAccessor = gltfModel.accessors[primitive.indices];
			const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
			const tinygltf::Buffer& indexBuffer = gltfModel.buffers[indexBufferView.buffer];

			bool uses32BitIndicies = false;
			
			if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				const uint16_t* indiceArray = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
				indices.Allocate(indexAccessor.count * sizeof(uint16_t));
				memcpy(indices.GetData(), indiceArray, indexAccessor.count * sizeof(uint16_t));
			}
			else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				const uint32_t* indiceArray = reinterpret_cast<const uint32_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
				indices.Allocate(indexAccessor.count * sizeof(uint32_t));
				memcpy(indices.GetData(), indiceArray, indexAccessor.count * sizeof(uint32_t));
				uses32BitIndicies = true;
			}
			else {
				SPDLOG_ERROR("Unsuppoerted index type");
				continue;
			}
			

			//Ref<Mesh> mesh = Ref<Mesh>::Make(gltfMesh.name, vertices, indices, StaticVertex::CreateLayout(), 0, uses32BitIndicies);

			vertices.Release();
			indices.Release();

			//meshes.push_back(mesh);
		}

		return meshes;
	}
}