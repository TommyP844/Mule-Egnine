#pragma once

#include "GraphicsContext.h"
#include "CommandPool.h"
#include "CommandBuffer.h"

#include "imguiImpl/imgui_impl_vulkan.h"

namespace Mule
{
	class ImGuiContext
	{
	public:
		ImGuiContext(Ref<GraphicsContext> graphicsContext);
		~ImGuiContext();

		void NewFrame();
		void EndFrame();
		void Resize(uint32_t width, uint32_t height);

		Ref<Fence> GetRenderSemaphore() const { return mSemaphore; }

	private:
		WeakRef<GraphicsContext> mContext;
		WeakRef<GraphicsQueue> mGraphicsQueue;
		Ref<CommandPool> mCommandPool;
		Ref<CommandBuffer> mCommandBuffer;
		Ref<SwapchainFrameBuffer> mFrameBuffer;
		Ref<Semaphore> mSemaphore;
		Ref<Fence> mFence;
		ImGui_ImplVulkanH_Window mImGuiData;
	};
}