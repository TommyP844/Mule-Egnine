#include "Rendering/Framebuffer.h"


namespace Mule
{
	FrameBuffer::FrameBuffer(const FramebufferDescription& desc)
		:
		mDesc(desc)
	{
	}

	FrameBuffer::~FrameBuffer()
	{

	}

	void FrameBuffer::Resize(int width, int height)
	{
		mDesc.Width = width;
		mDesc.Height = height;


	}
}