#include "Rendering/Framebuffer.h"


namespace Mule
{
	Framebuffer::Framebuffer(const FramebufferDesc& desc)
		:
		mDesc(desc)
	{
		CreateFramebuffer();
	}

	Framebuffer::~Framebuffer()
	{
		if (bgfx::isValid(mHandle))
		{
			bgfx::destroy(mHandle);
		}
	}

	void Framebuffer::Resize(int width, int height)
	{
		mDesc.Width = width;
		mDesc.Height = height;

		bgfx::destroy(mHandle);

		CreateFramebuffer();
	}

	void Framebuffer::Bind()
	{
		bgfx::setViewFrameBuffer(0, mHandle);
		bgfx::setViewRect(0, 0, 0, mDesc.Width, mDesc.Height);
	}

	void Framebuffer::CreateFramebuffer()
	{
		std::vector<bgfx::Attachment> attachments;

		for (int i = 0; i < mDesc.AttachmentFormats.size(); i++)
		{
			bgfx::TextureHandle handle = CreateAttachment(mDesc.Width, mDesc.Height, mDesc.AttachmentFormats[i]);
			bgfx::Attachment attachment;
			attachment.init(handle);
			mAttachments.push_back(handle);
			attachments.push_back(attachment);
		}

		mHandle = bgfx::createFrameBuffer(attachments.size(), attachments.data(), true);
	}

	bgfx::TextureHandle Framebuffer::CreateAttachment(int width, int height, const AttachmentDesc& attachmentDesc)
	{
		bgfx::TextureFormat::Enum internalFormat = (bgfx::TextureFormat::Enum)attachmentDesc.Format;

		int flags = BGFX_TEXTURE_RT;
		if (attachmentDesc.UsedforReadback)
			flags |= BGFX_TEXTURE_READ_BACK;

		bgfx::TextureHandle textureHandle = bgfx::createTexture2D(
			width, height,       
			false,             
			attachmentDesc.Layers,
			internalFormat,
			flags
		);
		return textureHandle;
	}
}