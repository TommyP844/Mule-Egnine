#pragma once

#include "Ref.h"
#include "RenderTypes.h"
#include "RenderPass.h"

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

	struct FramebufferDescription
	{
		int Width;
		int Height;
		int Layers;
		std::vector<AttachmentDesc> AttachmentFormats;
		Ref<RenderPass> RenderPass = nullptr;
	};

	class FrameBuffer
	{
	public:
		FrameBuffer(const FramebufferDescription& desc);
		~FrameBuffer();

		int GetWidth() const { return mDesc.Width; }
		int GetHeight() const { return mDesc.Height; }

		void Resize(int width, int height);


	private:
		FramebufferDescription mDesc;
	};
}