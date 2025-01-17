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
		RenderPass(VkDevice device, VkRenderPass renderPass) : mDevice(device), mRenderPass(renderPass) {}
		~RenderPass();

		VkRenderPass GetHandle() const { return mRenderPass; }

	private:
		VkDevice mDevice;
		VkRenderPass mRenderPass;
	};
}