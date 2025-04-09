#pragma once

#include "RenderTypes.h"

#include <Volk/volk.h>
#include <glm/glm.hpp>

namespace Mule
{
	struct SwapchainFrameBufferDescription
	{
		VkDevice Device = VK_NULL_HANDLE;
		VulkanImage ColorImage;
		VulkanImage DepthImage;
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		uint32_t Width;
		uint32_t Height;
	};

	class SwapchainFrameBuffer
	{
	public:
		SwapchainFrameBuffer(const SwapchainFrameBufferDescription& desc);
		~SwapchainFrameBuffer();

		VkFramebuffer GetHandle() const { return mFramebuffer; }
		VkRenderPass GetRenderPass() const { return mRenderPass; }

		uint32_t GetWidth() const { return mWidth; }
		uint32_t GetHeight() const { return mHeight; }

		void SetClearValue(int index, const glm::vec4& clearValue);
		void SetClearValue(int index, const glm::ivec4& clearValue);
		void SetClearValue(int index, const glm::uvec4& clearValue);
		std::vector<VkClearValue> GetClearValues() const { return mClearValues; }

		VulkanImage& GetColorImage() { return mColorImage; }
		VulkanImage& GetDepthImage() { return mDepthImage; }

	private:
		uint32_t mWidth, mHeight;
		VkDevice mDevice;
		VkFramebuffer mFramebuffer;
		VulkanImage mDepthImage;
		VulkanImage mColorImage;
		VkRenderPass mRenderPass;

		std::vector<VkClearValue> mClearValues;
	};
}