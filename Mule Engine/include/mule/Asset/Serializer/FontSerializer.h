#pragma once

#include "IAssetSerializer.h"

#include "Graphics/UI/UIFont.h"

namespace Mule
{
	class FontSerializer : public IAssetSerializer<UIFont, AssetType::UIFont>
	{
	public:
		FontSerializer(Ref<ServiceManager> serviceManager);
		~FontSerializer();

		Ref<UIFont> Load(const fs::path& filepath) override;
		void Save(Ref<UIFont> asset) override;
	};
}