#include "Graphics/API/Vulkan/VulkanFramebuffer.h"

// Submodule
#include <spdlog/spdlog.h>

// STD
#include <set>

namespace Mule::Vulkan
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferDescription& desc)
		:
		mDescription(desc)
	{
		mWidth = mDescription.Width;
		mHeight = mDescription.Height;
		mColorAttachmentCount = desc.ColorAttachments.size();
		Resize(mWidth, mHeight);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;

		// Release old framebuffer attachments
		mColorAttachments.clear();
		mDepthAttachment = nullptr;

		for (auto attachmentDescription : mDescription.ColorAttachments)
		{
			Ref<VulkanTexture2D> attachment = MakeRef<VulkanTexture2D>(
				attachmentDescription.Name,
				Buffer(),
				mWidth,
				mHeight,
				attachmentDescription.Format, 
				attachmentDescription.Flags | TextureFlags::RenderTarget
			);

			mColorAttachments.push_back(attachment);
		}

		if (mDescription.DepthAttachment.Format != TextureFormat::NONE)
		{
			mDepthAttachment = MakeRef<VulkanTexture2D>(
				"Depth Attachment",
				Buffer(),
				mWidth,
				mHeight,
				mDescription.DepthAttachment.Format,
				mDescription.DepthAttachment.Flags | TextureFlags::RenderTarget | TextureFlags::DepthAttachment
			);
		}
	}

	Ref<Texture2D> VulkanFramebuffer::GetColorAttachment(uint32_t index)
	{
		return mColorAttachments[index];
	}

	Ref<Texture2D> VulkanFramebuffer::GetDepthAttachment()
	{
		return mDepthAttachment;
	}

}
