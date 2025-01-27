#include "Graphics/Framebuffer.h"

#include "Graphics/Context/GraphicsContext.h"

// Submodule
#include <spdlog/spdlog.h>

// STD
#include <set>

namespace Mule
{
	FrameBuffer::FrameBuffer(WeakRef<GraphicsContext> context, const FramebufferDescription& desc)
		:
		mContext(context),
		mDesc(desc),
		mFrameBuffer(VK_NULL_HANDLE)
	{
		Resize(mDesc.Width, mDesc.Height);
	}

	FrameBuffer::~FrameBuffer()
	{
		Invalidate();
	}

	WeakRef<Texture2D> FrameBuffer::GetColorAttachment(int index)
	{
		return mColorAttachments[index];
	}

	WeakRef<Texture2D> FrameBuffer::GetDepthAttachment()
	{
		return mDepthAttachment;
	}

	void FrameBuffer::SetColorClearValue(int attachmentIndex, glm::vec4 clearColor)
	{
		mClearValues[attachmentIndex].color.float32[0] = clearColor.x;
		mClearValues[attachmentIndex].color.float32[1] = clearColor.y;
		mClearValues[attachmentIndex].color.float32[2] = clearColor.z;
		mClearValues[attachmentIndex].color.float32[3] = clearColor.w;
	}

	void FrameBuffer::SetColorClearValue(int attachmentIndex, glm::ivec4 clearColor)
	{
		mClearValues[attachmentIndex].color.int32[0] = clearColor.x;
		mClearValues[attachmentIndex].color.int32[1] = clearColor.y;
		mClearValues[attachmentIndex].color.int32[2] = clearColor.z;
		mClearValues[attachmentIndex].color.int32[3] = clearColor.w;
	}

	void FrameBuffer::SetColorClearValue(int attachmentIndex, glm::uvec4 clearColor)
	{
		mClearValues[attachmentIndex].color.uint32[0] = clearColor.x;
		mClearValues[attachmentIndex].color.uint32[1] = clearColor.y;
		mClearValues[attachmentIndex].color.uint32[2] = clearColor.z;
		mClearValues[attachmentIndex].color.uint32[3] = clearColor.w;
	}

	void FrameBuffer::SetDepthClearColor(float clearValue)
	{
		mClearValues.back().depthStencil.depth = clearValue;
	}

	void FrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		Invalidate();

		mDesc.Width = width;
		mDesc.Height = height;

		std::vector<VkImageView> framebufferViews;

		for (auto attachmentDesc : mDesc.Attachments)
		{
			Ref<Texture2D> attachment = MakeRef<Texture2D>(mContext, nullptr, mDesc.Width, mDesc.Height, 1, attachmentDesc.Format, TextureFlags::RenderTarget);
			mColorAttachments.push_back(attachment);
			framebufferViews.push_back(attachment->GetImageView());
		}

		if (mDesc.DepthAttachment.Format != TextureFormat::NONE)
		{
			mDepthAttachment = MakeRef<Texture2D>(mContext, nullptr, mDesc.Width, mDesc.Height, 1, mDesc.DepthAttachment.Format, (TextureFlags)(TextureFlags::RenderTarget | TextureFlags::DepthTexture));
			framebufferViews.push_back(mDepthAttachment->GetImageView());
		}

		if (mClearValues.empty())
		{
			mClearValues.resize(framebufferViews.size());
			for (int i = 0; i < mClearValues.size(); i++)
			{
				SetColorClearValue(i, glm::vec4(0.f, 0.f, 0.f, 1.f));
			}

			if (mDesc.DepthAttachment.Format != TextureFormat::NONE)
			{
				mClearValues.back().depthStencil.depth = 1.f;
			}
		}

		VkFramebufferCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = mDesc.Width;
		info.height = mDesc.Height;
		info.layers = mDesc.LayerCount;
		info.renderPass = mDesc.RenderPass->GetHandle();
		info.flags = 0;
		info.attachmentCount = framebufferViews.size();
		info.pAttachments = framebufferViews.data();
		info.pNext = nullptr;

		VkResult result = vkCreateFramebuffer(mContext->GetDevice(), &info, nullptr, &mFrameBuffer);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create framebuffer");
		}
	}

	void FrameBuffer::Invalidate()
	{
		mColorAttachments.clear();
		mDepthAttachment = nullptr;
		if (mFrameBuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(mContext->GetDevice(), mFrameBuffer, nullptr);
		}
	}

}