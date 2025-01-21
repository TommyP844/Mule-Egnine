#pragma once

#include "RenderTypes.h"

#include <vulkan/vulkan.h>

// STD
#include <vector>

namespace Mule
{
	

	struct RenderPassDescription
	{
		std::vector<Attachment> Attachments;
		Attachment DepthAttachment;
	};

	class RenderPass
	{
	public:
		RenderPass(VkDevice device, const RenderPassDescription& description);
		~RenderPass();

		VkRenderPass GetHandle() const { return mRenderPass; }

		const std::vector<Attachment> GetColorAttachments() const { return mDescription.Attachments; }

	private:
		RenderPassDescription mDescription;
		VkDevice mDevice;
		VkRenderPass mRenderPass;
	};
}