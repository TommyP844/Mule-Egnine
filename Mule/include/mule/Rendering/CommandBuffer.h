#pragma once

// Render Object
#include "Ref.h"
#include "RenderPass.h"
#include "SwapchainframeBuffer.h"

#include <vulkan/vulkan.h>

namespace Mule
{
	class CommandBuffer
	{
	public:
		CommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
		~CommandBuffer();
		void Begin();
		void End();

		void TransitionSwapchainFrameBufferForRendering(Ref<SwapchainFrameBuffer> fb);
		void TransitionSwapchainFrameBufferForPresent(Ref<SwapchainFrameBuffer> fb);

		// Framebuffers
		void BeginSwapChainFrameBuffer(Ref<SwapchainFrameBuffer> framebuffer);
		void EndFramebuffer();

		VkCommandBuffer GetHandle() const { return mCommandBuffer; }

	private:
		VkDevice mDevice;
		VkCommandPool mCommandPool;
		VkCommandBuffer mCommandBuffer;
	};
}