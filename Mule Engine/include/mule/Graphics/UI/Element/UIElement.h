#pragma once

#include "WeakRef.h"

#include "Graphics/UI/UIHandle.h"
#include "Graphics/Renderer/CommandList.h"
#include "Graphics/UI/UITransform.h"
#include "Graphics/UI/UIStyle.h"
#include "Graphics/UI/UITheme.h"

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

	constexpr UIElementType GetUIElementTypeFromString(const std::string& type)
	{
		if (type == "UIText") return UIElementType::UIText;
		if (type == "UIButton") return UIElementType::UIButton;
		return UIElementType::MAX_UI_ELEMENT_TYPE;
	}

	class UIElement
	{
	public:
		UIElement(const std::string& name, UIElementType elementType, UIHandle handle = UIHandle::Create());

		void SetName(const std::string& name) { mName = name; }
		const std::string& GetName() const { return mName; }

		// Hierarchy
		WeakRef<UIElement> GetParent() const { return mParent; }		
		void AddChild(const Ref<UIElement>& child);
		void RemoveChild(const Ref<UIElement>& child);
		const std::vector<Ref<UIElement>> GetChildren() const { return mChildren; }

		// Per Frame
		void Update(const UIRect& parentRect);
		virtual void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) = 0;

		// Syle
		void SetStyle(WeakRef<UIStyle> style) { mStyle = style; }
		WeakRef<UIStyle> GetStyle() const { return mStyle; }

		// Transform
		void SetTransform(const UITransform& transform) { mTransform = transform; mIsDirty = true; }
		const UITransform& GetTransform() const { return mTransform; }
		UITransform& GetTransform() { return mTransform; }
		
		// Helpers
		void SetVisible(bool visible) { mVisible = visible; }
		bool IsVisible() const { return mVisible; }
		const UIRect& GetScreenRect() const { return mScreenRect; }

		void SetHandle(UIHandle handle) { mHandle = handle; }
		UIHandle GetHandle() const { return mHandle; }

		WeakRef<UIElement> HitTest(float screenX, float screenY);
		UIElementType GetType() const { return mType; }

		template<typename T>
		WeakRef<T> As()
		{
			static_assert(std::is_base_of<UIElement, T>::value, "T must derive from UIElement");
			return WeakRef<T>((T*)this);
		}

	protected:
		WeakRef<UIStyle> mStyle;
		bool mVisible;
		UITransform mTransform;

		void MarkDirty() { mIsDirty = true; }

	private:
		std::string mName;
		bool mIsDirty;
		UIElementType mType;

		UIHandle mHandle;
		UIRect mScreenRect;
		WeakRef<UIElement> mParent;
		std::vector<Ref<UIElement>> mChildren;
	};
}