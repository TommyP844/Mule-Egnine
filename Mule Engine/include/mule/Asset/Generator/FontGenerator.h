#pragma once

#include "IAssetGenerator.h"

#include "Graphics/UI/UIFont.h"


namespace Mule
{
	class FontGenerator : public IAssetGenerator
	{
	public:
		FontGenerator(Ref<ServiceManager> serviceManager);
		~FontGenerator();

		Ref<UIFont> GenerateFont(const fs::path& fontFile);
	};
}
