#include "Graphics/RenderPass.h"

#include <spdlog/spdlog.h>

Mule::RenderPass::RenderPass(VkDevice device, const RenderPassDescription& description)
	:
	mDevice(device),
	mRenderPass(VK_NULL_HANDLE),
	mDescription(description)
{
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	std::vector<VkAttachmentReference> attachmentReferences;
	for (int i = 0; i < description.Attachments.size(); i++)
	{
		const Attachment& attachment = description.Attachments[i];

		VkAttachmentDescription attachmentDesc{};
		attachmentDesc.format = (VkFormat)attachment.Format;
		attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDesc.flags = 0;
		attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

		attachmentDescriptions.push_back(attachmentDesc);

		VkAttachmentReference attachmentRef{};

		attachmentRef.attachment = i;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachmentReferences.push_back(attachmentRef);
	}

	// Depth Attachment
	VkAttachmentReference depthAttachmentRef{};
	bool hasDepth = false;
	{
		if (description.DepthAttachment.Format != TextureFormat::NONE)
		{
			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = (VkFormat)description.DepthAttachment.Format;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			attachmentDesc.flags = 0;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

			attachmentDescriptions.push_back(attachmentDesc);

			hasDepth = true;
			depthAttachmentRef.attachment = description.Attachments.size();
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}

	}

	// Subpass
	std::vector<VkSubpassDescription> subpasses{};
	std::vector<std::vector<VkAttachmentReference>> attachmentRefs; // We need the memory to persist until the function ends
	attachmentRefs.resize(description.Subpasses.size());
	for(uint32_t i = 0; i < description.Subpasses.size(); i++)
	{
		const auto& subPassDesc = description.Subpasses[i];
		mSubPassAttachmentCount.push_back(subPassDesc.AttachmentRefs.size());
		for (uint32_t j = 0; j < subPassDesc.AttachmentRefs.size(); j++)
		{
			VkAttachmentReference attachmentRef{};

			attachmentRef.attachment = subPassDesc.AttachmentRefs[j];
			attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachmentRefs[i].push_back(attachmentRef);
		}

		VkSubpassDescription subpass{};
		subpass.colorAttachmentCount = attachmentReferences.size();
		subpass.pColorAttachments = attachmentReferences.data();
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pDepthStencilAttachment = subPassDesc.HasDepth ? &depthAttachmentRef : nullptr;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;
		subpass.pResolveAttachments = nullptr;;
		subpasses.push_back(subpass);
	}

	VkRenderPassCreateInfo renderPassCreateInfo{};

	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.subpassCount = subpasses.size();
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;
	renderPassCreateInfo.subpassCount = subpasses.size();
	renderPassCreateInfo.pSubpasses = subpasses.data();

	VkResult result = vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &mRenderPass);

	if (result == VK_SUCCESS)
	{
		SPDLOG_INFO("Render Pass Created");
	}
	else
	{
		SPDLOG_ERROR("Failed to create render pass");
		return;
	}
}

Mule::RenderPass::~RenderPass()
{
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
}
