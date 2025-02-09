#pragma once

#include "AssetLoader.h"
#include "Graphics/Model.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Asset/AssetManager.h"
#include "Graphics/Material.h"

#include <assimp/scene.h>

// STD
#include <vector>

namespace YAML
{
	class Node;
}

namespace Mule
{
	class ModelLoader : public IAssetLoader<Model, AssetType::Model>
	{
	public:
		ModelLoader(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager);
		virtual ~ModelLoader() {}

		Ref<Model> LoadText(const fs::path& filepath) override;
		void SaveText(Ref<Model> asset) override;

		Ref<Model> LoadBinary(const Buffer& filepath) override;
		void SaveBinary(Ref<Model> asset) override;

	private:
		WeakRef<AssetManager> mAssetManager;
		WeakRef<GraphicsContext> mGraphicsContext;

		struct LoadInfo
		{
			LoadInfo(const fs::path& filepath, const aiScene* scene)
				:
				Filepath(filepath),
				Scene(scene)
			{}

			const aiScene* Scene;
			const fs::path& Filepath;

			int TextureCount = 0;
			int MaterialCount = 0;
			int MeshCount = 0;
			std::map<std::string, AssetHandle> Textures;
			std::map<std::string, AssetHandle> Materials;
			std::map<std::string, AssetHandle> Meshes;
		};

		void RecurseNodes(const aiNode* ainode, ModelNode& node, LoadInfo& info);
		Ref<Mesh> LoadMesh(const aiMesh* mesh, LoadInfo& info);
		Ref<Material> LoadMaterial(const aiMaterial* material, LoadInfo& info);
		Ref<Texture2D> LoadTexture(const aiTexture* texture);
		Ref<Texture2D> LoadTexture(const aiMaterial* material, const std::vector<aiTextureType>& textureTypes, LoadInfo& info);
		std::string CreateAssetName(std::string name, LoadInfo& info, AssetType assetType);

		void LoadSerializationInfo(const fs::path& metaPath, LoadInfo& info);
		void BuildSerializationInfo(const fs::path& metaPath, Ref<Model> model);
		void RecurseModelInfo(const ModelNode& node, YAML::Node& meshes, YAML::Node& materials, YAML::Node& textures);
	};
}