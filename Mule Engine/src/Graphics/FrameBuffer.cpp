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
		mDesc(desc)
	{
		mClearValues.resize(mDesc.Attachments.size());
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


		for (auto attachmentDesc : mDesc.Attachments)
		{
			TextureFlags flags = (TextureFlags)((uint32_t)TextureFlags::RenderTarget | (uint32_t)attachmentDesc.Flags);
			Ref<Texture2D> attachment = MakeRef<Texture2D>(mContext, nullptr, mDesc.Width, mDesc.Height, mDesc.LayerCount, attachmentDesc.Format, flags);
			mColorAttachments.push_back(attachment);
		}

		if (mDesc.DepthAttachment.Format != TextureFormat::NONE)
		{
			mDepthAttachment = MakeRef<Texture2D>(mContext, nullptr, mDesc.Width, mDesc.Height, mDesc.LayerCount, mDesc.DepthAttachment.Format, (TextureFlags)(TextureFlags::RenderTarget | TextureFlags::DepthTexture));
		}
	}

	void FrameBuffer::Invalidate()
	{
		mColorAttachments.clear();
		mDepthAttachment = nullptr;
	}

}