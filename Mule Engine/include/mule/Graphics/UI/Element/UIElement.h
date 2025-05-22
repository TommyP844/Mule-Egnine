#pragma once

#include "WeakRef.h"

#include "Graphics/UI/UIHandle.h"
#include "Graphics/Renderer/CommandList.h"
#include "Graphics/UI/UITransform.h"
#include "Graphics/UI/UIStyle.h"
#include "Graphics/UI/UITheme.h"
#include "Graphics/UI/UIAnchor.h"

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

		void AddAnchor(WeakRef<UIElement> targetElement, UIAnchorAxis targetAxis, UIAnchorAxis selfAxis);
		void RemoveAnchor(UIAnchorAxis selfAxis);

		void SetName(const std::string& name) { mName = name; }
		const std::string& GetName() const { return mName; }

		// Per Frame
		virtual void Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) = 0;
		virtual void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) = 0;

		// Style
		void SetStyle(WeakRef<UIStyle> style) { mStyle = style; }
		WeakRef<UIStyle> GetStyle() const { return mStyle; }

		// Transform
		void SetTransform(const UITransform& transform) { mTransform = transform; mIsDirty = true; }
		const UITransform& GetTransform() const { return mTransform; }
		UITransform& GetTransform() { return mTransform; }
		void SetLeft(float value, UIUnitType type);
		void SetRight(float value, UIUnitType type);
		void SetTop(float value, UIUnitType type);
		void SetBottom(float value, UIUnitType type);
		void SetWidth(float value, UIUnitType type);
		void SetHeight(float value, UIUnitType type);

		// Helpers
		void SetVisible(bool visible) { mVisible = visible; }
		bool IsVisible() const { return mVisible; }
		void UpdateRect(const UIRect& parentRect);
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

		bool mIsDirty;
		UIRect mScreenRect;

		// Self Axis -> Anchor
		std::unordered_map<UIAnchorAxis, UIAnchor> mAnchors;

	private:
		std::string mName;
		UIElementType mType;

		UIHandle mHandle;
	};
}