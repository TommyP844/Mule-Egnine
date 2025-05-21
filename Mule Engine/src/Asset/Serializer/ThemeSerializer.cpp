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
		Ref<UITheme> theme = MakeRef<UITheme>("");
		theme->SetFilePath(filepath);

		YAML::Node node = YAML::LoadFile(filepath.string());

		for (const auto& it : node)
		{
			const std::string& name = it.first.as<std::string>();
			const YAML::Node& value = it.second;

			UIStyleKey key = GetUIStyleKeyFromName(name);
			if (key == UIStyleKey::STYLE_KEY_MAX)
				continue;

			UIStyleKeyDataType dataType = GetUIStyleKeyDataType(key);

			switch (dataType)
			{
			case Mule::UIStyleKeyDataType::Bool:		theme->SetValue(key, value.as<bool>()); break;
			case Mule::UIStyleKeyDataType::Integer:		theme->SetValue(key, value.as<int>()); break;
			case Mule::UIStyleKeyDataType::Float:		theme->SetValue(key, value.as<float>()); break;
			case Mule::UIStyleKeyDataType::Vec2:		theme->SetValue(key, value.as<glm::vec2>()); break;
			case Mule::UIStyleKeyDataType::Vec3:		theme->SetValue(key, value.as<glm::vec3>()); break;
			case Mule::UIStyleKeyDataType::Color:		theme->SetValue(key, value.as<glm::vec4>()); break;
			case Mule::UIStyleKeyDataType::Ivec2:		theme->SetValue(key, value.as<glm::ivec2>()); break;
			case Mule::UIStyleKeyDataType::IVec3:		theme->SetValue(key, value.as<glm::ivec3>()); break;
			case Mule::UIStyleKeyDataType::Ivec4:		theme->SetValue(key, value.as<glm::ivec4>()); break;
			case Mule::UIStyleKeyDataType::AssetHandle: theme->SetValue(key, value.as<AssetHandle>()); break;
			default:
				assert("Invalid UIStyleKeyDataType");
				break;
			}

		}

		return theme;
	}

	void ThemeSerializer::Save(Ref<UITheme> asset)
	{
		YAML::Node node;

		for (uint32_t i = 0; i < static_cast<uint32_t>(UIStyleKey::STYLE_KEY_MAX); i++)
		{
			UIStyleKey key = static_cast<UIStyleKey>(i);
			UIStyleKeyDataType dataType = GetUIStyleKeyDataType(key);
			std::string keyName = GetUIStyleKeyName(key);

			switch (dataType)
			{
			case Mule::UIStyleKeyDataType::Bool:		node[keyName] = asset->GetValue<bool>(key); break;
			case Mule::UIStyleKeyDataType::Integer:		node[keyName] = asset->GetValue<int>(key); break;
			case Mule::UIStyleKeyDataType::Float:		node[keyName] = asset->GetValue<float>(key); break;
			case Mule::UIStyleKeyDataType::Vec2:		node[keyName] = asset->GetValue<glm::vec2>(key); break;
			case Mule::UIStyleKeyDataType::Vec3:		node[keyName] = asset->GetValue<glm::vec3>(key); break;
			case Mule::UIStyleKeyDataType::Color:		node[keyName] = asset->GetValue<glm::vec4>(key); break;
			case Mule::UIStyleKeyDataType::Ivec2:		node[keyName] = asset->GetValue<glm::ivec2>(key); break;
			case Mule::UIStyleKeyDataType::IVec3:		node[keyName] = asset->GetValue<glm::ivec3>(key); break;
			case Mule::UIStyleKeyDataType::Ivec4:		node[keyName] = asset->GetValue<glm::ivec4>(key); break;
			case Mule::UIStyleKeyDataType::AssetHandle:	node[keyName] = asset->GetValue<AssetHandle>(key); break;
			}
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