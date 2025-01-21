#pragma once

#include "Ref.h"
#include "RenderTypes.h"
#include "RenderPass.h"
#include "WeakRef.h"

// STD
#include <vector>

namespace Mule
{
	class GraphicsContext;

	struct AttachmentDesc
	{
		TextureFormat Format = TextureFormat::NONE;
	};

	struct FramebufferDescription
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t LayerCount = 1;
		std::vector<AttachmentDesc> Attachments;
		AttachmentDesc DepthAttachment;
		Ref<RenderPass> RenderPass = nullptr;
	};

	class FrameBuffer
	{
	public:
		FrameBuffer(WeakRef<GraphicsContext> context, const FramebufferDescription& desc);
		~FrameBuffer();

		int GetWidth() const { return mDesc.Width; }
		int GetHeight() const { return mDesc.Height; }

		void Resize(uint32_t width, uint32_t height);


	private:
		void Invalidate();
		bool mIsValid;
		WeakRef<GraphicsContext> mContext;
		VkFramebuffer mFrameBuffer;
		FramebufferDescription mDesc;

		std::vector<VulkanImage> mColorAttachments;
		VulkanImage mDepthAttachment;

		bool CreateImage(VulkanImage& image, uint32_t width, uint32_t height, uint32_t layers, uint32_t mips, VkFormat format, VkImageViewType viewtype, VkImageUsageFlagBits usage, bool depthImage);
	};
}