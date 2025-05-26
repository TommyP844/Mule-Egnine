#include "Asset/Serializer/ThemeSerializer.h"

#include "yaml-cpp/yaml.h"
#include "Asset/Serializer/Convert/YamlConvert.h"

#include <fstream>

namespace Mule
{
	ThemeSerializer::ThemeSerializer(Ref<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	ThemeSerializer::~ThemeSerializer()
	{
	}

	Ref<UITheme> ThemeSerializer::Load(const fs::path& filepath)
	{
		Ref<UITheme> theme = MakeRef<UITheme>();
		theme->SetFilePath(filepath);



		return theme;
	}

	void ThemeSerializer::Save(Ref<UITheme> asset)
	{
		YAML::Node node;


		YAML::Emitter emitter;
		emitter << node;

		std::ofstream file(asset->FilePath());
		if (!file)
			return;

		file << emitter.c_str();

		file.close();
	}
}