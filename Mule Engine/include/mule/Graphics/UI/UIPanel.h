#pragma once

#include "Graphics/UI/UIRect.h"
#include "Graphics/UI/Element/UIElement.h"
#include "Graphics/Renderer/CommandList.h"

namespace Mule
{
	class UIPanel
	{
	public:
		UIPanel(const std::string& name);

		void AddElement(Ref<UIElement> element);
		void RemoveElement(Ref<UIElement> element);

		void Render(CommandList& commandList, const UIRect& windowRect);

		const std::string& GetName() const { return mName; }

	private:
		std::string mName;
		bool mVisible;
		std::vector<Ref<UIElement>> mElements;
	};
}