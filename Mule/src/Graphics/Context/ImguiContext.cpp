#include "Graphics/Context/ImGuiContext.h"

// Submodules
#include "Graphics/imguiImpl/imgui_impl_vulkan.h"
#include "Graphics/imguiImpl/imgui_impl_glfw.h"
#include "ImGuizmo.h"

namespace Mule
{
	static void check_vk_result(VkResult err)
	{
		if (err == VK_SUCCESS)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	ImGuiContext::ImGuiContext(Ref<GraphicsContext> graphicsContext)
		:
		mContext(graphicsContext),
		mFrameIndex(0)
	{
		mGraphicsQueue = graphicsContext->GetGraphicsQueue();
		mFrameBuffer = graphicsContext->GetSwapchainFrameBuffer();

		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].CommandPool = mGraphicsQueue->CreateCommandPool();
			mFrameData[i].CommandBuffer = mFrameData[i].CommandPool->CreateCommandBuffer();
			mFrameData[i].Fence = graphicsContext->CreateFence();
			mFrameData[i].Semaphore = graphicsContext->CreateSemaphore();
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.DisplaySize.x = graphicsContext->GetWindow()->GetWidth();
		io.DisplaySize.y = graphicsContext->GetWindow()->GetHeight();

		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

		ImGui_ImplGlfw_InitForVulkan(graphicsContext->GetWindow()->GetGLFWWindow(), true);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = graphicsContext->GetInstance();
		initInfo.PhysicalDevice = graphicsContext->GetPhysicalDevice();
		initInfo.Device = graphicsContext->GetDevice();
		initInfo.DescriptorPool = graphicsContext->GetDescriptorPool();
		initInfo.Queue = graphicsContext->GetGraphicsQueue()->GetHandle();
		initInfo.QueueFamily = graphicsContext->GetGraphicsQueue()->GetQueueFamilyIndex();
		initInfo.ImageCount = 2;
		initInfo.MinImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.Subpass = 0;
		initInfo.UseDynamicRendering = false;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = check_vk_result;
		initInfo.RenderPass = mFrameBuffer->GetRenderPass();
		ImGui_ImplVulkan_Init(&initInfo);

		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
		ImGui::GetStyle().Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 0.40f);
		ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 0.75f);
		ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.62f, 1.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.28f, 0.37f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.35f, 0.44f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0.37f, 0.44f, 0.53f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SeparatorActive] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_DockingPreview] = ImVec4(0.28f, 0.56f, 1.00f, 0.70f);
		ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.90f, 0.55f, 0.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.28f, 0.56f, 1.00f, 0.35f);
		ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.35f);

		ImGui::GetStyle().FrameRounding = 5.f;

	}

	ImGuiContext::~ImGuiContext()
	{
		vkDeviceWaitIdle(mContext->GetDevice());
		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].CommandBuffer = nullptr;
			mFrameData[i].CommandPool = nullptr;
		}
		ImGui_ImplVulkan_Shutdown();
	}

	void ImGuiContext::NewFrame()
	{
		// We put this here so when users grab the rendering finished semaphore they get the right one
		mFrameIndex ^= 1;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiContext::EndFrame(const std::vector<Ref<Semaphore>>& waitSemaphores)
	{
		FrameData& frameData = mFrameData[mFrameIndex];
		mFrameBuffer->SetClearValue(0, glm::vec4(0.f, 0.f, 0.f, 1.f));
		mFrameBuffer = mContext->GetSwapchainFrameBuffer();
		
		frameData.Fence->Wait();
		frameData.Fence->Reset();
		frameData.CommandPool->Reset();
		frameData.CommandBuffer->Begin();
		frameData.CommandBuffer->TransitionSwapchainFrameBufferForRendering(mFrameBuffer);
		frameData.CommandBuffer->BeginRenderPass(mFrameBuffer);

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(drawData, frameData.CommandBuffer->GetHandle());

		frameData.CommandBuffer->EndRenderPass();
		frameData.CommandBuffer->TransitionSwapchainFrameBufferForPresent(mFrameBuffer);
		frameData.CommandBuffer->End();
		mGraphicsQueue->Submit(frameData.CommandBuffer, waitSemaphores, { frameData.Semaphore }, frameData.Fence);

		ImGui::UpdatePlatformWindows();
	}

	void ImGuiContext::Resize(uint32_t width, uint32_t height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = mContext->GetWindow()->GetWidth();
		io.DisplaySize.y = mContext->GetWindow()->GetHeight();
	}
}