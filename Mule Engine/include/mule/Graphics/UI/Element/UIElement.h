#pragma once

#include "WeakRef.h"

#include "Graphics/Renderer/CommandList.h"
#include "Graphics/UI/UITransform.h"
#include "Graphics/UI/UIStyle.h"

#include <string>

namespace Mule
{
	enum class UIElementType
	{
		UIText,
		UIButton,

		MAX_UI_ELEMENT_TYPE
	};

	constexpr std::string GetUIElementNameFromType(UIElementType type)
	{
		switch (type)
		{
		case Mule::UIElementType::UIText: return "UIText";
		case Mule::UIElementType::UIButton: return "UIButton";
		case Mule::UIElementType::MAX_UI_ELEMENT_TYPE:
		default:
			assert("Invalid UIElementType");
			break;
		}
	}

	class UIElement
	{
	public:
		UIElement(const std::string& name = "UI Element");

		const std::string& GetName() const { return mName; }

		WeakRef<UIElement> GetParent() const { return mParent; }
		
		void AddChild(const Ref<UIElement>& child);
		void RemoveChild(const Ref<UIElement>& child);
		const std::vector<Ref<UIElement>> GetChildren() const { return mChildren; }

		virtual void Render(CommandList& commandList, const UIRect& parentRect) = 0;

		void SetStyle(Ref<UIStyle> style) { mStyle = style; }
		Ref<UIStyle> GetStyle() const { return mStyle; }

		const UITransform& GetTransform() const { return mTransform; }
		UITransform& GetTransform() { return mTransform; }
		
		bool IsVisible() const { return mVisible; }

	protected:
		Ref<UIStyle> mStyle;
		bool mVisible;
		UITransform mTransform;

	private:
		std::string mName;

		WeakRef<UIElement> mParent;
		std::vector<Ref<UIElement>> mChildren;
	};
}