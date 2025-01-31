#pragma once

#include "AssetLoader.h"
#include "Graphics/Model.h"
#include "tiny_gltf.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Asset/AssetManager.h"
#include "Graphics/Material.h"

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

		virtual Ref<Model> LoadBinary(const Buffer& filepath) override;
		virtual void SaveBinary(Ref<Model> asset) override;

		void SetGraphicsContext(WeakRef<GraphicsContext> context) { mGraphicsContext = context; }
		void SetAssetManager(WeakRef<AssetManager> assetManager) { mAssetManager = assetManager; }

	private:
		WeakRef<AssetManager> mAssetManager;
		WeakRef<GraphicsContext> mGraphicsContext;
		Ref<Model> ConvertModel(const tinygltf::Model& model, const fs::path& filepath);
		ModelNode ConvertNode(const tinygltf::Model& gltfModel, const tinygltf::Node& gltfNode, const fs::path& filepath);
		std::vector<Ref<Mesh>> ConvertMesh(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh, const fs::path& filepath);
		Ref<Material> LoadMaterial(const tinygltf::Model& gltfModel, const tinygltf::Material& gltfMaterial, const fs::path& filepath);
		Ref<Texture2D> LoadTexture(const tinygltf::Model& gltfModel, const tinygltf::TextureInfo& textureInfo, const fs::path& filepath);
		Ref<Texture2D> LoadTexture(const tinygltf::Model& gltfModel, const tinygltf::NormalTextureInfo& textureInfo, const fs::path& filepath);
		Ref<Texture2D> LoadTexture(const tinygltf::Model& gltfModel, const tinygltf::OcclusionTextureInfo& textureInfo, const fs::path& filepath);
		std::pair<Ref<Texture2D>, Ref<Texture2D>> LoadMetallicRoughnessTextures(const tinygltf::Model& gltfModel, const tinygltf::Material& material, const fs::path& filepath);

		template<typename T>
		const T* GetBufferAsArray(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, const std::string& name);

	};
}

#include "ModelLoader.inl"