#include "Rendering/GraphicsContext.h"

#include "spdlog/spdlog.h"


#include "imgui_impl_glfw.h"

namespace Mule
{
	GraphicsContext* GraphicsContext::sRenderer = nullptr;

	GraphicsContext::GraphicsContext(Ref<Window> window)
	{

	}

	GraphicsContext::~GraphicsContext()
	{

	}


	void GraphicsContext::NewFrame()
	{
	}

	void GraphicsContext::RenderFrame()
	{
	}

	void GraphicsContext::NewImGuiFrame()
	{
		ImGui::NewFrame();
	}

	void GraphicsContext::RenderImGuiFrame()
	{

	}
}
