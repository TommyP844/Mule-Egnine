#pragma once

#include "AssetLoader.h"
#include "Rendering/Model.h"
#include "tiny_gltf.h"

// STD
#include <vector>

namespace Mule
{
	class ModelLoader : public IAssetLoader<Model, AssetType::Model>
	{
	public:
		ModelLoader() {}
		virtual ~ModelLoader() {}

		virtual Ref<Model> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<Model> asset) override;

		virtual Ref<Model> LoadBinary(const fs::path& filepath) override;
		virtual void SaveBinary(Ref<Model> asset) override;

	private:
		Ref<Model> ConvertModel(const tinygltf::Model& model);
		ModelNode ConvertNode(const tinygltf::Model& gltfModel, const tinygltf::Node& gltfNode);
		std::vector<Ref<Mesh>> ConvertMesh(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh);

		template<typename T>
		std::vector<T> GetBufferAsVector(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, const std::string& name);
	};
}

#include "ModelLoader.inl"