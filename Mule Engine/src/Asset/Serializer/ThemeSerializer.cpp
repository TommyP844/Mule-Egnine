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

		YAML::Node node = YAML::LoadFile(filepath.string());

		theme->TextStyle = MakeRef<UITextStyle>();
		for(auto child : node["TextStyle"])
		{
			std::string stateName = child.first.as<std::string>();
			UIElementState state = FromString<UIElementState>(stateName);

			auto& vars = theme->TextStyle->GetStateVars(state);
			vars = child.second.as<UITextStyleVars>();
		}

		return theme;
	}

	void ThemeSerializer::Save(Ref<UITheme> asset)
	{
		YAML::Node node;

		// Text
		{
			YAML::Node textStyleNode = node["TextStyle"];
			SerializeStyle(asset->TextStyle, textStyleNode);
		}

		// Button
		{
			YAML::Node buttonStyleNode = node["ButtonStyle"];
			SerializeStyle(asset->ButtonStyle, buttonStyleNode);

			
			YAML::Node buttonTextStyleNode = buttonStyleNode["TextStyle"];
			SerializeStyle(asset->ButtonStyle->TextStyle.GetValue(), buttonTextStyleNode);
		}

		YAML::Emitter emitter;
		emitter << node;

		std::ofstream file(asset->FilePath());
		if (!file)
			return;

		file << emitter.c_str();

		file.close();
	}
}