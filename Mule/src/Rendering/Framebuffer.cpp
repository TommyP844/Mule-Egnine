#include "Rendering/Framebuffer.h"


namespace Mule
{
	Framebuffer::Framebuffer(const FramebufferDesc& desc)
		:
		mDesc(desc)
	{
	}

	Framebuffer::~Framebuffer()
	{

	}

	void Framebuffer::Resize(int width, int height)
	{
		mDesc.Width = width;
		mDesc.Height = height;


	}
}