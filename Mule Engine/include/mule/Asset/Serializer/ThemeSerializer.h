#pragma once

#include "IAssetSerializer.h"

#include "Graphics/UI/UITheme.h"

namespace Mule
{
	class ThemeSerializer : public IAssetSerializer<UITheme, AssetType::UITheme>
	{
	public:
		ThemeSerializer(Ref<ServiceManager> serviceManager);
		virtual ~ThemeSerializer();

		Ref<UITheme> Load(const fs::path& filepath) override;
		void Save(Ref<UITheme> asset) override;
	};
}
