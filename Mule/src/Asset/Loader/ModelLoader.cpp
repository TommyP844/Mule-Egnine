
#include "Asset/Loader/ModelLoader.h"

#include "Graphics/Vertex.h"

#include <assimp/Importer.hpp>
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
		Assimp::Importer importer = Assimp::Importer();
		const aiScene* scene = importer.ReadFile(filepath.string(), 0);
		Ref<Model> engineModel;

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
}