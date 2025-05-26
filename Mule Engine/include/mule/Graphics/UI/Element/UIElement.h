#pragma once

#include "WeakRef.h"

#include "Graphics/UI/Element/UIHandle.h"
#include "Graphics/UI/Element/UIAnchor.h"
#include "Graphics/UI/Element/UIElementType.h"
#include "Graphics/UI/Element/UIElementState.h"
#include "Graphics/UI/UITransform.h"
#include "Graphics/UI/UITheme.h"
#include "Graphics/Renderer/CommandList.h"

#include "Asset/AssetManager.h"

#include <string>

namespace Mule
{
	class UIScene;

	class UIBaseElement
	{
	public:
		UIBaseElement(const std::string& name, UIElementType elementType, UIHandle handle);
		virtual ~UIBaseElement() {}

		virtual void Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) = 0;
		virtual void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) = 0;

		// Anchors
		void AddAnchor(UIHandle targetElement, UIAnchorAxis targetAxis, UIAnchorAxis selfAxis);
		void RemoveAnchor(UIAnchorAxis selfAxis);
		void RemoveAllAnchors();
		bool IsAnchoredToElementAxis(UIHandle handle, UIAnchorAxis anchorAxis) const;
		const std::unordered_map<UIAnchorAxis, UIAnchor>& GetAnchors() const { return mAnchors; }

		void SetName(const std::string& name) { mName = name; }
		const std::string& GetName() const { return mName; }

		// Per Frame

		void OnEvent(WeakRef<Event> event);

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

		virtual void SetHandle(UIHandle handle) = 0;
		UIHandle GetHandle() const { return mHandle; }

		WeakRef<UIBaseElement> HitTest(float screenX, float screenY);
		UIElementType GetType() const { return mType; }

		template<typename T>
		WeakRef<T> As()
		{
			static_assert(std::is_base_of<UIBaseElement, T>::value, "T must derive from UIElement");
			return WeakRef<T>((T*)this);
		}

		virtual void SetScene(WeakRef<UIScene> scene) = 0;

	protected:
		UIElementState mState;
		bool mVisible;
		UITransform mTransform;

		bool mIsDirty;
		UIRect mScreenRect;

		// Self Axis -> Anchor
		std::unordered_map<UIAnchorAxis, UIAnchor> mAnchors;
		WeakRef<UIScene> mScene;

		UIHandle mHandle;
	private:
		std::string mName;
		UIElementType mType;
	};

	template<typename Style>
	class UIElement : public UIBaseElement
	{
	public:
		UIElement(const std::string& name, UIElementType elementType, UIHandle handle = UIHandle::Create());
		
		// Style
		void SetStyle(WeakRef<Style> style) { mStyle = style; }
		WeakRef<Style> GetStyle() const { return mStyle; }
				
	protected:
		WeakRef<Style> mStyle;

	};


	template<typename Style>
	inline UIElement<Style>::UIElement(const std::string& name, UIElementType elementType, UIHandle handle)
		:
		UIBaseElement(name, elementType, handle)
	{
	}
}