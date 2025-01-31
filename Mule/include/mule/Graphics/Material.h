#pragma once

#include "Asset/Asset.h"

#include <glm/glm.hpp>

namespace Mule
{
	class Material : public Asset<AssetType::Material>
	{
	public:
		Material() : Asset() {}

		glm::vec4 AlbedoColor = glm::vec4(1.f);

		glm::vec2 TextureScale = glm::vec2(1.f);

		bool Transparent = false;

		float Transparency = 1.f;
		float MetalnessFactor = 0.f;
		float RoughnessFactor = 0.f;
		float AOFactor = 0.f;

		AssetHandle AlbedoMap = NullAssetHandle;
		AssetHandle NormalMap = NullAssetHandle;
		AssetHandle MetalnessMap = NullAssetHandle;
		AssetHandle RoughnessMap = NullAssetHandle;
		AssetHandle AOMap = NullAssetHandle;
		AssetHandle EmissiveMap = NullAssetHandle;
	};
}