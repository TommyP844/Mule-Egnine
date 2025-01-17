#include "Rendering/ImGuiContext.h"

// Submodules
#include "Rendering/imguiImpl/imgui_impl_vulkan.h"
#include "Rendering/imguiImpl/imgui_impl_glfw.h"

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
		mContext(graphicsContext)
	{
		mFrameBuffer = graphicsContext->GetSwapchainFrameBuffer();
		mGraphicsQueue = graphicsContext->GetGraphicsQueue();
		mCommandPool = mGraphicsQueue->CreateCommandPool();
		mCommandBuffer = mCommandPool->CreateCommandBuffer();
		mFence = graphicsContext->CreateFence();
		mSemaphore = graphicsContext->CreateSemaphore();

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
	}

	ImGuiContext::~ImGuiContext()
	{
		ImGui_ImplVulkan_Shutdown();
	}

	void ImGuiContext::NewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiContext::EndFrame()
	{
		mFrameBuffer->SetClearValue(0, glm::vec4(0.f, 0.f, 0.f, 1.f));
		mFrameBuffer = mContext->GetSwapchainFrameBuffer();
		
		mFence->Wait();
		mFence->Reset();
		mCommandPool->Reset();
		mCommandBuffer->Begin();
		mCommandBuffer->TransitionSwapchainFrameBufferForRendering(mFrameBuffer);
		mCommandBuffer->BeginSwapChainFrameBuffer(mFrameBuffer);

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(drawData, mCommandBuffer->GetHandle());

		mCommandBuffer->EndFramebuffer();
		mCommandBuffer->TransitionSwapchainFrameBufferForPresent(mFrameBuffer);
		mCommandBuffer->End();
		mGraphicsQueue->Submit(mCommandBuffer, {}, { mSemaphore }, mFence);

		ImGui::UpdatePlatformWindows();
	}

	void ImGuiContext::Resize(uint32_t width, uint32_t height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = mContext->GetWindow()->GetWidth();
		io.DisplaySize.y = mContext->GetWindow()->GetHeight();
	}
}