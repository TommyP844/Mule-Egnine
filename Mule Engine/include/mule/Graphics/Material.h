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
		float MetalnessFactor = 1.f;
		float RoughnessFactor = 1.f;
		float AOFactor = 1.f;

		AssetHandle AlbedoMap = AssetHandle::Null();
		AssetHandle NormalMap = AssetHandle::Null();
		AssetHandle MetalnessMap = AssetHandle::Null();
		AssetHandle RoughnessMap = AssetHandle::Null();
		AssetHandle AOMap = AssetHandle::Null();
		AssetHandle EmissiveMap = AssetHandle::Null();
		AssetHandle OpacityMap = AssetHandle::Null();

		uint32_t GlobalIndex;
	};
}