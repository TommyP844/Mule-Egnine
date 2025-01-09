#include "Rendering/Renderer.h"

#include "spdlog/spdlog.h"

#include "bgfx/bgfx.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "imgui_impl_bgfx.h"
#include "imgui_impl_glfw.h"

namespace Mule
{

	Renderer* Renderer::sRenderer = nullptr;

	Renderer& Renderer::Init(Ref<Window> window)
	{
		assert(sRenderer == nullptr && "Renderer already initialized");
		sRenderer = new Renderer(window);
		return *sRenderer;
	}

	void Renderer::Shutdown()
	{
		SPDLOG_INFO("Shutting down renderer...");
		delete sRenderer;
		SPDLOG_INFO("Renderer shutdown");
	}

	Renderer& Renderer::Get()
	{
		return *sRenderer;
	}

	void Renderer::NewFrame()
	{
		bgfx::touch(0);
	}

	void Renderer::RenderFrame()
	{
		bgfx::frame();
	}

	void Renderer::NewImGuiFrame()
	{
		ImGui_Implbgfx_NewFrame();
		ImGui::NewFrame();
	}

	void Renderer::RenderImGuiFrame()
	{
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

		bgfx::TextureHandle;
	}

	Renderer::Renderer(Ref<Window> window)
	{
		bgfx::PlatformData pd{};
		pd.nwh = glfwGetWin32Window(window->GetGLFWWindow());
		pd.ndt = nullptr;

		bgfx::Init init;
		init.type = bgfx::RendererType::Direct3D12; // Auto-detect renderer
		init.resolution.width = window->GetWidth();
		init.resolution.height = window->GetHeight();
		init.resolution.reset = BGFX_RESET_VSYNC;
		init.debug = true;
		init.platformData = pd;
		//init.callback = &callback;
		if (!bgfx::init(init))
		{
			SPDLOG_CRITICAL("Failed to init renderer");
			exit(1);
		}
		SPDLOG_INFO("Renderer Initialized");

		bgfx::RendererType::Enum rendererType = bgfx::getRendererType();

		SPDLOG_INFO("\tAPI: {}", bgfx::getRendererName(rendererType));

		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);

		// ImGui

		SPDLOG_INFO("ImGui initializing...");

		ImGui_Implbgfx_Init(0);
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOther(window->GetGLFWWindow(), true);

		SPDLOG_INFO("ImGui initialized");
		
		auto& io = ImGui::GetIO(); (void)io;
		io.DisplaySize.x = window->GetWidth();
		io.DisplaySize.y = window->GetHeight();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	}

	Renderer::~Renderer()
	{
		ImGui_Implbgfx_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		bgfx::shutdown();
	}
}
