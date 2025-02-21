#pragma once

#include "AssetLoader.h"

#include "Graphics/Material.h"

namespace Mule
{
	class MaterialLoader : public IAssetLoader<Material, AssetType::Material>
	{
	public:
		MaterialLoader(){}
		~MaterialLoader(){}

		// Inherited via IAssetLoader
		Ref<Material> LoadText(const fs::path& filepath) override;

		void SaveText(Ref<Material> asset) override;

		Ref<Material> LoadBinary(const Buffer& buffer) override;

		void SaveBinary(Ref<Material> asset) override;

	};
}