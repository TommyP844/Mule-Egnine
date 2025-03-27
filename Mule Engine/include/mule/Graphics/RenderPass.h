#pragma once

#include "RenderTypes.h"

#include <vulkan/vulkan.h>

// STD
#include <vector>

namespace Mule
{
	struct SubPassDesc
	{
		std::vector<uint32_t> AttachmentRefs;
		bool HasDepth;
	};

	struct RenderPassDescription
	{
		std::vector<Attachment> Attachments;
		Attachment DepthAttachment;

		std::vector<SubPassDesc> Subpasses;
	};

	class RenderPass
	{
	public:
		RenderPass(VkDevice device, const RenderPassDescription& description);
		~RenderPass();

		VkRenderPass GetHandle() const { return mRenderPass; }

		const std::vector<Attachment> GetColorAttachments() const { return mDescription.Attachments; }
		uint32_t SubPassAttachmentCount(uint32_t subPass) const { return mSubPassAttachmentCount[subPass]; }

	private:
		RenderPassDescription mDescription;
		std::vector<uint32_t> mSubPassAttachmentCount;
		VkDevice mDevice;
		VkRenderPass mRenderPass;
	};
}