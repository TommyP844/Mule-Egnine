#pragma once

#include "WeakRef.h"

#include "Graphics/Renderer/CommandList.h"
#include "Graphics/UI/UITransform.h"
#include "Graphics/UI/UIStyle.h"

#include "Asset/AssetManager.h"

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
		UIElement(const std::string& name, UIElementType elementType);

		void SetName(const std::string& name) { mName = name; }
		const std::string& GetName() const { return mName; }

		// Hierarchy
		WeakRef<UIElement> GetParent() const { return mParent; }		
		void AddChild(const Ref<UIElement>& child);
		void RemoveChild(const Ref<UIElement>& child);
		const std::vector<Ref<UIElement>> GetChildren() const { return mChildren; }

		// Per Frame
		void Update(const UIRect& parentRect);
		virtual void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager) = 0;

		// Syle
		void SetStyle(WeakRef<UIStyle> style) { mStyle = style; }
		WeakRef<UIStyle> GetStyle() const { return mStyle; }

		// Transform
		const UITransform& GetTransform() const { return mTransform; }
		UITransform& GetTransform() { return mTransform; }
		
		// Helpers
		void SetVisible(bool visible) { mVisible = visible; }
		bool IsVisible() const { return mVisible; }
		const UIRect& GetScreenRect() const { return mScreenRect; }

		WeakRef<UIElement> HitTest(float screenX, float screenY);

		UIElementType GetType() const { return mType; }
	protected:
		WeakRef<UIStyle> mStyle;
		bool mVisible;
		UITransform mTransform;

		void MarkDirty() { mIsDirty = true; }

	private:
		std::string mName;
		bool mIsDirty;
		UIElementType mType;

		UIRect mScreenRect;
		WeakRef<UIElement> mParent;
		std::vector<Ref<UIElement>> mChildren;
	};
}