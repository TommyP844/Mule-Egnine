#include "Rendering/SwapchainFrameBuffer.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	SwapchainFrameBuffer::SwapchainFrameBuffer(const SwapchainFrameBufferDescription& desc)
		:
		mDepthImage(desc.DepthImage),
		mColorImage(desc.ColorImage),
		mDevice(desc.Device),
		mFramebuffer(VK_NULL_HANDLE),
		mRenderPass(desc.RenderPass),
		mWidth(desc.Width),
		mHeight(desc.Height)
	{

		std::vector<VkImageView> attachmentsViews = {
			desc.ColorImage.ImageView
		};

		if (desc.DepthImage.ImageView != VK_NULL_HANDLE) {
			attachmentsViews.push_back(desc.DepthImage.ImageView);
		}


		for (int i = 0; i < attachmentsViews.size(); i++)
		{
			VkClearValue cv{};
			cv.color.float32[0] = 0.f;
			cv.color.float32[1] = 0.f;
			cv.color.float32[2] = 0.f;
			cv.color.float32[3] = 0.f;

			cv.depthStencil.depth = 1.f;

			mClearValues.push_back(cv);
		}

		VkFramebufferCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = desc.Width;
		info.height = desc.Height;
		info.layers = 1;
		info.renderPass = desc.RenderPass;
		info.flags = 0;
		info.attachmentCount = attachmentsViews.size();
		info.pAttachments = attachmentsViews.data();
		info.pNext = nullptr;

		VkResult result = vkCreateFramebuffer(mDevice, &info, nullptr, &mFramebuffer);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create Swapchain Frame Buffer");
		}
	}

	SwapchainFrameBuffer::~SwapchainFrameBuffer()
	{
		if (mDepthImage.Image != VK_NULL_HANDLE)
		{
			vkDestroyImageView(mDevice, mDepthImage.ImageView, nullptr);
			vkFreeMemory(mDevice, mDepthImage.Memory, nullptr);
			vkDestroyImage(mDevice, mDepthImage.Image, nullptr);
		}
		vkDestroyImageView(mDevice, mColorImage.ImageView, nullptr);
		vkDestroyFramebuffer(mDevice, mFramebuffer, nullptr);
	}

	void SwapchainFrameBuffer::SetClearValue(int index, const glm::vec4& clearValue)
	{
		mClearValues[index].color.float32[0] = clearValue.x;
		mClearValues[index].color.float32[1] = clearValue.y;
		mClearValues[index].color.float32[2] = clearValue.z;
		mClearValues[index].color.float32[3] = clearValue.w;
	}

	void SwapchainFrameBuffer::SetClearValue(int index, const glm::ivec4& clearValue)
	{
		mClearValues[index].color.int32[0] = clearValue.x;
		mClearValues[index].color.int32[1] = clearValue.y;
		mClearValues[index].color.int32[2] = clearValue.z;
		mClearValues[index].color.int32[3] = clearValue.w;
	}

	void SwapchainFrameBuffer::SetClearValue(int index, const glm::uvec4& clearValue)
	{
		mClearValues[index].color.uint32[0] = clearValue.x;
		mClearValues[index].color.uint32[1] = clearValue.y;
		mClearValues[index].color.uint32[2] = clearValue.z;
		mClearValues[index].color.uint32[3] = clearValue.w;
	}
}