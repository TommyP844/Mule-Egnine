#include "Graphics/UI/UITheme.h"

#include "Engine Context/EngineAssets.h"

namespace Mule
{
	UITheme::UITheme(const std::string& name)
		:
		Asset(name)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(UIStyleKey::STYLE_KEY_MAX); i++)
		{
			UIStyleKey key = static_cast<UIStyleKey>(i);
			UIStyleKeyDataType dataType = GetUIStyleKeyDataType(key);
			switch (dataType)
			{
			case Mule::UIStyleKeyDataType::Bool:
				SetValue(key, false);
				break;
			case Mule::UIStyleKeyDataType::Integer:
				SetValue(key, 0);
				break;
			case Mule::UIStyleKeyDataType::Float:
				SetValue(key, 0.f);
				break;
			case Mule::UIStyleKeyDataType::Vec2:
				SetValue(key, glm::vec2(0.f));
				break;
			case Mule::UIStyleKeyDataType::Vec3:
				SetValue(key, glm::vec3(1.f));
				break;
			case Mule::UIStyleKeyDataType::Color:
				SetValue(key, glm::vec4(1.f));
				break;
			case Mule::UIStyleKeyDataType::Ivec2:
				SetValue(key, glm::ivec2(0));
				break;
			case Mule::UIStyleKeyDataType::IVec3:
				SetValue(key, glm::ivec3(0));
				break;
			case Mule::UIStyleKeyDataType::Ivec4:
				SetValue(key, glm::ivec4(0));
				break;
			case Mule::UIStyleKeyDataType::AssetHandle:
				SetValue(key, AssetHandle::Null());
				break;
			default:
				break;
			}
		}

		SetValue(UIStyleKey::Font, AssetHandle(MULE_DEFAULT_FONT_HANDLE));
		SetValue(UIStyleKey::FontSize, 25.f);
	}

	UITheme::~UITheme()
	{
	}


	Ref<UITheme> UITheme::GetDefault()
	{
		static Ref<UITheme> theme = nullptr;
		if (!theme)
		{
			theme = MakeRef<UITheme>("Default");
			theme->SetValue(UIStyleKey::Font, AssetHandle(MULE_DEFAULT_FONT_HANDLE));
			theme->SetValue(UIStyleKey::FontSize, 25.f);
		}

		return theme;
	}
}