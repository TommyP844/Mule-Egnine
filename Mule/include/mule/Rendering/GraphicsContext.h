#pragma once

#include "Ref.h"
#include "Application/Window.h"

namespace Mule
{
	class GraphicsContext
	{
	public:
		GraphicsContext(Ref<Window> window);
		~GraphicsContext();
		
		GraphicsContext(const GraphicsContext&) = delete;
		
		void NewFrame();
		void RenderFrame();

		void NewImGuiFrame();
		void RenderImGuiFrame();

	private:
		
	};
}