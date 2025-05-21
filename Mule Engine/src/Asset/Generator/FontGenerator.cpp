#include "Asset/Generator/FontGenerator.h"



namespace Mule
{
	FontGenerator::FontGenerator(Ref<ServiceManager> serviceManager)
		:
		IAssetGenerator(serviceManager)
	{
	}

	FontGenerator::~FontGenerator()
	{
	}

	Ref<UIFont> FontGenerator::GenerateFont(const fs::path& fontFile)
	{
		return nullptr;
	}
}