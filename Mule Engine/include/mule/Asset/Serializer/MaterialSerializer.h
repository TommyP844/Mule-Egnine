#pragma once

#include "IAssetSerializer.h"

#include "Graphics/Material.h"

namespace Mule
{
	class MaterialSerializer : public IAssetSerializer<Material, AssetType::Material>
	{
	public:
		MaterialSerializer(Ref<ServiceManager> serviceManager);
		~MaterialSerializer(){}

		// Inherited via IAssetLoader
		Ref<Material> Load(const fs::path& filepath) override;

		void Save(Ref<Material> asset) override;

	};
}