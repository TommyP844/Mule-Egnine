#pragma once

#include "AssetLoader.h"
#include "Rendering/Model.h"

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
	};
}