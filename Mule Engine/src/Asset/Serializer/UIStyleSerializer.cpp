#include "Asset/Serializer/UIStyleSerializer.h"

#include "Asset/Serializer/Convert/YamlConvert.h"

#include "yaml-cpp/yaml.h"

#include <fstream>

namespace Mule
{
	UIStyleSerializer::UIStyleSerializer(Ref<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	Ref<UIStyle> UIStyleSerializer::Load(const fs::path& filepath)
	{
		Ref<UIStyle> style = MakeRef<UIStyle>("");
		style->SetFilePath(filepath);

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
			case Mule::UIStyleKeyDataType::Bool:		style->SetValue(key, value.as<bool>()); break;
			case Mule::UIStyleKeyDataType::Integer:		style->SetValue(key, value.as<int>()); break;
			case Mule::UIStyleKeyDataType::Float:		style->SetValue(key, value.as<float>()); break;
			case Mule::UIStyleKeyDataType::Vec2:		style->SetValue(key, value.as<glm::vec2>()); break;
			case Mule::UIStyleKeyDataType::Vec3:		style->SetValue(key, value.as<glm::vec3>()); break;
			case Mule::UIStyleKeyDataType::Color:		style->SetValue(key, value.as<glm::vec4>()); break;
			case Mule::UIStyleKeyDataType::Ivec2:		style->SetValue(key, value.as<glm::ivec2>()); break;
			case Mule::UIStyleKeyDataType::IVec3:		style->SetValue(key, value.as<glm::ivec3>()); break;
			case Mule::UIStyleKeyDataType::Ivec4:		style->SetValue(key, value.as<glm::ivec4>()); break;
			case Mule::UIStyleKeyDataType::AssetHandle: style->SetValue(key, value.as<AssetHandle>()); break;
			default:
				assert("Invalid UIStyleKeyDataType");
				break;
			}

		}

		return style;
	}

	void UIStyleSerializer::Save(Ref<UIStyle> asset)
	{
		YAML::Node node;

		for (uint32_t i = 0; i < static_cast<uint32_t>(UIStyleKey::STYLE_KEY_MAX); i++)
		{
			UIStyleKey key = static_cast<UIStyleKey>(i);
			UIStyleKeyDataType dataType = GetUIStyleKeyDataType(key);
			std::string keyName = GetUIStyleKeyName(key);

			if (!asset->HasValue(key))
				continue;

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