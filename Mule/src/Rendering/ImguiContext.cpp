#include "Rendering/ImGuiContext.h"

// Submodules
#include "Rendering/imguiImpl/imgui_impl_vulkan.h"
#include "Rendering/imguiImpl/imgui_impl_glfw.h"

namespace Mule
{
	ImGuiContext::ImGuiContext(Ref<GraphicsContext> graphicsContext)
		:
		mContext(graphicsContext)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


		ImGui_ImplGlfw_InitForVulkan(graphicsContext->GetWindow()->GetGLFWWindow(), true);
				
		ImGui_ImplVulkan_InitInfo initInfo;
		initInfo.Instance = graphicsContext->GetInstance();
		initInfo.PhysicalDevice = graphicsContext->GetPhysicalDevice();
		initInfo.Device = graphicsContext->GetDevice();
		initInfo.DescriptorPool = graphicsContext->GetDescriptorPool();
		initInfo.Queue = graphicsContext->GetGraphicsQueue()->GetHandle();
		initInfo.QueueFamily = graphicsContext->GetGraphicsQueue()->GetQueueFamilyIndex();
		initInfo.ColorAttachmentFormat = VK_FORMAT_R8G8B8A8_SNORM;
		initInfo.ImageCount = 2;
		initInfo.MinImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.Subpass = 0;
		initInfo.UseDynamicRendering = false;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = nullptr;
		
		// TODO: create render pass
		ImGui_ImplVulkan_Init(&initInfo, nullptr);

		mGraphicsQueue = graphicsContext->GetGraphicsQueue();
		mCommandPool = mGraphicsQueue->CreateCommandPool();
		mCommandBuffer = mCommandPool->CreateCommandbuffer();

		mCommandBuffer->Begin();
		ImGui_ImplVulkan_CreateFontsTexture(mCommandBuffer->GetHandle());
		mCommandBuffer->End();
		mGraphicsQueue->Submit(mCommandBuffer);
		ImGui_ImplVulkan_DestroyFontUploadObjects();       
	}
	void ImGuiContext::NewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiContext::EndFrame()
	{
		ImGui::Render();
		mCommandBuffer->Begin();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mCommandBuffer->GetHandle());
		mCommandBuffer->End();
		mGraphicsQueue->Submit(mCommandBuffer);
	}
}