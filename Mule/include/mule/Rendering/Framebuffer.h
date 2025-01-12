#pragma once

#include "RenderTypes.h"

#include <bgfx/bgfx.h>

// STD
#include <vector>

namespace Mule
{
	struct AttachmentDesc
	{
		TextureFormat Format;
		int Layers = 1;
		bool UsedforReadback = false;
	};

	struct FramebufferDesc
	{
		int Width;
		int Height;
		int Layers;
		std::vector<AttachmentDesc> AttachmentFormats;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferDesc& desc);
		~Framebuffer();

		int GetWidth() const { return mDesc.Width; }
		int GetHeight() const { return mDesc.Height; }

		void Resize(int width, int height);

		void Bind();

	private:
		FramebufferDesc mDesc;
		std::vector<bgfx::TextureHandle> mAttachments;
		bgfx::FrameBufferHandle mHandle;

		void CreateFramebuffer();
		bgfx::TextureHandle CreateAttachment(int width, int height, const AttachmentDesc& attachmentDesc);
	};
}