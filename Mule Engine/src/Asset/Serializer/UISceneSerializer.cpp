#include "Asset/Serializer/UISceneSerializer.h"

#include "yaml-cpp/yaml.h"
#include "Asset/Serializer/Convert/YamlConvert.h"
#include "Asset/Serializer/Convert/YamlUIConvert.h"

// Elements
#include "Graphics/UI/Element/UIText.h"

#include <fstream>
#include <spdlog/spdlog.h>

namespace Mule
{
	UISceneSerializer::UISceneSerializer(Ref<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	UISceneSerializer::~UISceneSerializer()
	{
	}

	Ref<UIScene> UISceneSerializer::Load(const fs::path& filepath)
	{
		Ref<UIScene> scene = MakeRef<UIScene>();
		scene->SetFilePath(filepath);

		YAML::Node node = YAML::LoadFile(filepath.string());

		AssetHandle themeHandle = node["Theme"].as<AssetHandle>();
		scene->SetThemeHandle(themeHandle);
		for (auto child : node["Elements"])
		{
			auto type = GetUIElementTypeFromString(child["Type"].as<std::string>());
			Ref<UIElement> element;
			switch (type)
			{
			case UIElementType::UIText:
				element = child.as<Ref<UIText>>();
				break;
			}

			scene->AddUIElement(element);
		}

		return scene;
	}

	void UISceneSerializer::Save(Ref<UIScene> scene)
	{
		YAML::Node node;

		node["Theme"] = scene->GetThemeHandle();
		
		for (auto element : scene->GetUIElements())
		{
			YAML::Node elementNode = SerializeElement(element);
			node["Elements"].push_back(elementNode);
		}

		YAML::Emitter emitter;
		emitter << node;

		fs::path filePath = scene->FilePath();
		std::ofstream file(filePath);
		if (!file)
		{
			SPDLOG_ERROR("Failed to open UI Scene file: {}", filePath.string());
			return;
		}

		file << emitter.c_str();
		file.close();
	}
	
	YAML::Node UISceneSerializer::SerializeElement(Ref<UIElement> element)
	{
		YAML::Node elementNode;

		switch (element->GetType())
		{
		case UIElementType::UIText:
			elementNode = Ref<UIText>(element);
			break;
		default:
			assert(false && "Invalid UIElementType");
			break;
		}		

		for (auto child : element->GetChildren())
		{
			YAML::Node childNode = SerializeElement(child);
			elementNode["Children"].push_back(childNode);
		}

		return elementNode;
	}

}