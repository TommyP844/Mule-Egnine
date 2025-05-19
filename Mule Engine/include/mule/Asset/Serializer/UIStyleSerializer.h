#pragma once

#include "Graphics/UI/UIStyle.h"

#include "IAssetSerializer.h"

namespace Mule
{
	class UIStyleSerializer : public IAssetSerializer<UIStyle, AssetType::UIStyle>
	{
	public:
		UIStyleSerializer(Ref<ServiceManager> serviceManager);

		// Inherited via IAssetSerializer
		Ref<UIStyle> Load(const fs::path& filepath) override;
		void Save(Ref<UIStyle> asset) override;
	};
}
