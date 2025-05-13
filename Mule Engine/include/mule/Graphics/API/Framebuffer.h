#pragma once

#include "Ref.h"
#include "Texture2D.h"

#include <vector>

namespace Mule
{
	struct FramebufferAttachment
	{
		FramebufferAttachment() = default;
		FramebufferAttachment(TextureFormat format, TextureFlags flags, const std::string& name = "Attachment")
			:
			Format(format),
			Flags(flags),
			Name(name)
		{ }

		// We default this value so on framebuffer creation we can tell if the users wanted a depth attachment or not
		TextureFormat Format = TextureFormat::NONE;
		TextureFlags Flags;
		std::string Name = "Color Attachment";
	};

	struct FramebufferDescription
	{
		FramebufferDescription() = default;
		uint32_t Width;
		uint32_t Height;
		uint32_t Layers = 1;
		std::vector<FramebufferAttachment> ColorAttachments;
		FramebufferAttachment DepthAttachment;
	};

	class Framebuffer
	{
	public:
		static Ref<Framebuffer> Create(const FramebufferDescription& description);

		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual Ref<Texture2D> GetColorAttachment(uint32_t index) = 0;
		virtual Ref<Texture2D> GetDepthAttachment() = 0;
		
		uint32_t GetWidth() const { return mWidth; }
		uint32_t GetHeight() const { return mHeight; }
		uint32_t GetColorAttachmentCount() const { return mColorAttachmentCount; }

	protected:
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mColorAttachmentCount;
	};
}
