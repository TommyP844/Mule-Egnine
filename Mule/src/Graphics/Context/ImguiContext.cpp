#include "Graphics/Context/ImGuiContext.h"

// Submodules
#include "Graphics/imguiImpl/imgui_impl_vulkan.h"
#include "Graphics/imguiImpl/imgui_impl_glfw.h"

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
		frameData.CommandBuffer->BeginSwapChainFrameBuffer(mFrameBuffer);

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(drawData, frameData.CommandBuffer->GetHandle());

		frameData.CommandBuffer->EndFramebuffer();
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