#pragma once

#include "AssetLoader.h"
#include "Graphics/Model.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Asset/AssetManager.h"
#include "Graphics/Material.h"
#include "Graphics/Model.h"

#include <assimp/scene.h>

// STD
#include <vector>

namespace Mule
{
	class ModelLoader : public IAssetLoader<Model, AssetType::Model>
	{
	public:
		ModelLoader(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager);
		virtual ~ModelLoader() {}

		virtual Ref<Model> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<Model> asset) override;

		virtual Ref<Model> LoadBinary(const Buffer& filepath) override;
		virtual void SaveBinary(Ref<Model> asset) override;


	private:
		WeakRef<AssetManager> mAssetManager;
		WeakRef<GraphicsContext> mGraphicsContext;

		void RecurseNodes(const aiScene* scene, const aiNode* ainode, ModelNode& node, const fs::path& filepath);
		Ref<Mesh> LoadMesh(const aiScene* scene, const aiMesh* mesh, const fs::path& filepath);
		Ref<Material> LoadMaterial(const aiScene* scene, const aiMaterial* material, const fs::path& filepath);
		Ref<Texture2D> LoadTexture(const aiScene* scene, const aiTexture* texture, const fs::path& filepath);
	};
}