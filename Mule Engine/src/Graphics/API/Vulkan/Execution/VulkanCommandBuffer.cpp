#include "Graphics/API/Vulkan/Execution/VulkanCommandBuffer.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include <spdlog/spdlog.h>

// API
#include "Graphics/API/Vulkan/VulkanFrameBuffer.h"
#include "Graphics/API/Vulkan/Pipeline/VulkanComputePipeline.h"
#include "Graphics/API/Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Graphics/API/Vulkan/Buffer/VulkanUniformBuffer.h"
#include "Graphics/API/Vulkan/Texture/IVulkanTexture.h"
#include "Graphics/API/Vulkan/Buffer/IVulkanBuffer.h"
#include "Graphics/API/Vulkan/VulkanDescriptorSet.h"
#include "Graphics/API/Vulkan/Buffer/VulkanIndexBuffer.h"
#include "Graphics/API/Vulkan/Buffer/VulkanVertexBuffer.h"
#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

#include "Graphics/API/Vulkan/VulkanTypeConversion.h"

#include <Volk/volk.h>

#include <vector>

namespace Mule::Vulkan
{
	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool commandPool)
		:
		mCommandPool(commandPool)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		VkCommandBufferAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = mCommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		vkAllocateCommandBuffers(device, &allocInfo, &mCommandBuffer);
	}

	//TODO: remove vkDeviceWaitIdle and use a fence
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkFreeCommandBuffers(device, mCommandPool, 1, &mCommandBuffer);
	}

	void VulkanCommandBuffer::Reset()
	{
		vkResetCommandBuffer(mCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void VulkanCommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo beginInfo{};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;
		beginInfo.pNext = nullptr;

		vkBeginCommandBuffer(mCommandBuffer, &beginInfo);
	}

	void VulkanCommandBuffer::End()
	{
		vkEndCommandBuffer(mCommandBuffer);
	}

	void VulkanCommandBuffer::BeginSwapchainRendering()
	{
		VulkanContext& context = VulkanContext::Get();
		WeakRef<Window> window = context.GetWindow();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = context.GetCurrentSwapchainColorImage();
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		// Access masks: preserve existing content and prepare for render target use
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(
			mCommandBuffer,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // Usually safe for present-to-render
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		VkRenderingAttachmentInfo colorAttachment{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = context.GetCurrentSwapchainColorImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = VkClearValue{},
		};

		VkRenderingAttachmentInfo depthAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = context.GetCurrentSwapchainDepthImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = VkClearValue{},
		};

		VkRect2D rect{};
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = window->GetWidth();
		rect.extent.height = window->GetHeight();

		VkRenderingInfo info{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderArea = rect,
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = 1u,
			.pColorAttachments = &colorAttachment,
			.pDepthAttachment = &depthAttachmentInfo,
			.pStencilAttachment = nullptr,
		};

		vkCmdBeginRenderingKHR(mCommandBuffer, &info);

		VkViewport viewport{
			.x = 0,
			.y = static_cast<float>(window->GetHeight()),
			.width = static_cast<float>(window->GetWidth()),
			.height = static_cast<float>(-window->GetHeight()),
			.minDepth = 0.f,
			.maxDepth = 1.f,
		};

		vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(mCommandBuffer, 0, 1, &rect);
	}

	void VulkanCommandBuffer::EndSwapchainRendering()
	{
		VulkanContext& context = VulkanContext::Get();

		vkCmdEndRenderingKHR(mCommandBuffer);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = context.GetCurrentSwapchainColorImage();
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			mCommandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VulkanCommandBuffer::BeginRendering(WeakRef<Framebuffer> framebuffer, WeakRef<GraphicsPipeline> shader, const std::vector<WeakRef<ShaderResourceGroup>>& groups)
	{
		VkRect2D rect{};
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = framebuffer->GetWidth();
		rect.extent.height = framebuffer->GetHeight();

		WeakRef<VulkanFramebuffer> vulkanFramebuffer = framebuffer;
		WeakRef<VulkanGraphicsPipeline> vulkanPipeline = shader;

		std::vector<VkRenderingAttachmentInfo> colorAttachments;

		std::vector<ShaderAttachment> shaderAttachments = shader->GetOutputAttachments();

		for (uint32_t i = 0; i < vulkanFramebuffer->GetColorAttachmentCount(); i++)
		{
			WeakRef<VulkanTexture2D> attachment = framebuffer->GetColorAttachment(i);

			VkImageView view = attachment->GetVulkanImage().ImageView;

			if (std::find_if(shaderAttachments.begin(), shaderAttachments.end(), [i](const ShaderAttachment& n){ return n.Location == i; }) == shaderAttachments.end())
			{
				view = VK_NULL_HANDLE;
			}

			VkRenderingAttachmentInfo colorAttachment{};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.imageView = view;
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = VkClearValue{};
			colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
			colorAttachments.push_back(colorAttachment);
		}

		bool hasDepth = false;
		VkRenderingAttachmentInfo depthAttachmentInfo{};
		if (framebuffer->GetDepthAttachment())
		{
			WeakRef<VulkanTexture2D> depthAttachment = vulkanFramebuffer->GetDepthAttachment();

			depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachmentInfo.imageView = depthAttachment->GetVulkanImage().ImageView;
			depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachmentInfo.clearValue.depthStencil.depth = 1.f;
			depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;

			hasDepth = true;
		}

		VkRenderingInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.renderArea = rect;
		info.layerCount = 1;
		info.colorAttachmentCount = colorAttachments.size();
		info.pColorAttachments = colorAttachments.data();
		info.pDepthAttachment = hasDepth ? &depthAttachmentInfo : nullptr;
		info.pStencilAttachment = nullptr;
		info.pNext = nullptr;
		info.viewMask = 0;
		info.flags = 0;

		vkCmdBeginRenderingKHR(mCommandBuffer, &info);

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = framebuffer->GetHeight();
		viewport.width = framebuffer->GetWidth();
		viewport.height = -((float)framebuffer->GetHeight());
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(mCommandBuffer, 0, 1, &rect);

		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetPipeline());

		if (groups.size() > 0)
		{
			std::vector<VkDescriptorSet> sets(groups.size());
			for (uint32_t i = 0; i < groups.size(); i++)
			{
				WeakRef<VulkanDescriptorSet> descriptorSet = groups[i];
				sets[i] = descriptorSet->GetDescriptorSet();
			}
			vkCmdBindDescriptorSets(
				mCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vulkanPipeline->GetPipelineLayout(),
				0,
				sets.size(),
				sets.data(),
				0,
				nullptr
			);
		}
	}

	void VulkanCommandBuffer::ClearFrameBuffer(WeakRef<Framebuffer> framebuffer)
	{
		WeakRef<VulkanFramebuffer> vulkanFrambuffer = framebuffer;

		for (uint32_t i = 0; i < vulkanFrambuffer->GetColorAttachmentCount(); i++)
		{
			WeakRef<VulkanTexture2D> attachment = vulkanFrambuffer->GetColorAttachment(i);

			VkImageSubresourceRange range = {};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkClearColorValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };

			VkImageLayout oldLayout = attachment->GetVulkanImage().Layout;
			TranistionImageLayout(attachment, ImageLayout::TransferDst);

			vkCmdClearColorImage(
				mCommandBuffer,
				attachment->GetVulkanImage().Image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				&clearColor,
				1,
				&range
			);

			TranistionImageLayout(attachment, GetImageLayout(oldLayout));
		}

		WeakRef<VulkanTexture2D> depthAttachment = framebuffer->GetDepthAttachment();
		if (depthAttachment)
		{
			VkImageSubresourceRange range = {};
			range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkClearDepthStencilValue clearColor;
			clearColor.depth = 1.f;

			VkImageLayout oldLayout = depthAttachment->GetVulkanImage().Layout;
			TranistionImageLayout(depthAttachment, ImageLayout::TransferDst);

			vkCmdClearDepthStencilImage(
				mCommandBuffer,
				depthAttachment->GetVulkanImage().Image,
				depthAttachment->GetVulkanImage().Layout,
				&clearColor,
				1,
				&range
			);

			TranistionImageLayout(depthAttachment, GetImageLayout(oldLayout));
		}
	}

	void VulkanCommandBuffer::EndRendering()
	{
		vkCmdEndRenderingKHR(mCommandBuffer);
	}

	// TODO: this only supports 2d textures, no arrays, cubes, or 3d images
	void VulkanCommandBuffer::TranistionImageLayout(WeakRef<Texture> texture, ImageLayout newLayout)
	{
		WeakRef<VulkanTexture2D> vulkanTexture = texture;

		VkImageLayout oldLayout = vulkanTexture->GetVulkanImage().Layout;
		VkImageLayout newVkLayout = GetImageLayout(newLayout);

		if (oldLayout == newVkLayout)
			return;

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		VkImageAspectFlags imageAspect = vulkanTexture->GetIsDepthTexture() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		vulkanTexture->SetImageLayout(newVkLayout);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newVkLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = vulkanTexture->GetVulkanImage().Image;
		barrier.subresourceRange.aspectMask = imageAspect;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = vulkanTexture->GetImageMipLevels();
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = vulkanTexture->GetImageArrayLayers();
		

		if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newVkLayout ==VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_GENERAL)
		{
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; // Suitable for storage images

			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newVkLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Used for sampling in shaders

			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newVkLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Used for sampling in shaders

			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;  // Shader read occurs in the fragment stage
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;         // Preparing for transfer

			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // Previously used for reading in shaders
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // Now used for transfer reads
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;          // Source is a transfer operation
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;   // Destination is shader read (commonly in fragment stage)

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // Ensure transfer read is completed
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;   // Allow shader to read the image
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;  // Previously used in a shader (commonly fragment shader)
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;         // Now used for transfer operations

			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;  // Ensure all shader reads are completed
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Ensure the transfer operation can safely write to the image
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;          // Previously used for transfer operations
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;   // Now will be used for shader sampling

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Ensure all transfer writes are completed
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;    // Allow shaders to safely read the image
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;  // Ensures depth writes are completed
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;      // Prepares for shader sampling

			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // Ensure depth writes finish
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;                    // Allow shaders to read depth
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;      // Ensure shaders are done reading
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Prepare for depth writes

			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;      // Ensure all shader reads are finished
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // Allow depth writing
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_GENERAL)
		{
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // Ensure color writes are finished
			dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;           // Prepare for compute shader access

			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // Ensure all writes are finished
			barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT; // Allow compute shader access
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newVkLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;       // Ensure compute shader writes are finished
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;      // Prepare for fragment shader read

			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;    // Ensure compute shader writes are finished
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;     // Allow fragment shader to read
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newVkLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;       // Ensure compute shader writes are finished
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Prepare for color attachment output

			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;    // Ensure compute shader writes are finished
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Ensure proper writes to color attachment
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;  // Ensure depth writes are completed
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;             // Prepare for transfer operations

			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // Ensure depth writes are finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;                 // Allow writing via transfer operations

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Target depth aspect
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;               // Ensure all transfers are complete
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // Prepare for depth attachment usage

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;    // Ensure transfer writes are finished
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // Allow depth attachment writes

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Target depth aspect
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // Ensure color attachment writes are finished
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;                 // Prepare for transfer operations

			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // Ensure color writes are completed
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;           // Allow transfer reads

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Target color aspect
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;   // Wait for all color attachment writes
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;                  // Prepare for transfer operations

			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Make sure color writes are finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;         // We want to write via transfer (e.g., vkCmdClearColorImage)

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // This is a color image
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newVkLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;                      // Wait for transfer writes to complete
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;      // Prepare for color attachment usage

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;          // Ensure transfer write is finished
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // We will write as a color attachment

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // This is a color image
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newVkLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;                  // No need to wait on anything
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;     // Prepare for color attachment usage

			barrier.srcAccessMask = 0;                                     // No need to wait for previous accesses
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // We will write as a color attachment

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // This is a color image
			}
		else
		{
			SPDLOG_ERROR("Invalid layout transition");
		}

		vkCmdPipelineBarrier(
			mCommandBuffer,
			srcStage,
			dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vulkanTexture->SetImageLayout(newVkLayout);
	}

	void VulkanCommandBuffer::CopyTexture(WeakRef<Texture> src, WeakRef<Texture> dst, const TextureCopyInfo& copyInfo) const
	{
		WeakRef<VulkanTexture2D> vulkanSrc = src;
		WeakRef<VulkanTexture2D> vulkanDst = dst;

		VkImageCopy region{};
		region.srcOffset.x = copyInfo.SrcOffset.x;
		region.srcOffset.y = copyInfo.SrcOffset.y;
		region.srcOffset.z = copyInfo.SrcOffset.z;
		
		region.dstOffset.x = copyInfo.DstOffset.x;
		region.dstOffset.y = copyInfo.DstOffset.y;
		region.dstOffset.z = copyInfo.DstOffset.z;

		region.extent.width = copyInfo.Extent.x;
		region.extent.height = copyInfo.Extent.y;
		region.extent.depth = copyInfo.Extent.z;

		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.baseArrayLayer = copyInfo.SrcArrayLayer;
		region.srcSubresource.layerCount = copyInfo.SrcArrayLayerCount;
		region.srcSubresource.mipLevel = copyInfo.SrcMipLevel;

		region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.baseArrayLayer = copyInfo.DstArrayLayer;
		region.dstSubresource.layerCount = copyInfo.DstArrayLayerCount;
		region.dstSubresource.mipLevel = copyInfo.DstMipLevel;
		
		vkCmdCopyImage(mCommandBuffer, vulkanSrc->GetVulkanImage().Image, vulkanSrc->GetVulkanImage().Layout, vulkanDst->GetVulkanImage().Image, vulkanDst->GetVulkanImage().Layout, 1, &region);
	}

	void VulkanCommandBuffer::ReadTexture(WeakRef<Texture> texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, WeakRef<StagingBuffer> buffer) const
	{
		WeakRef<VulkanTexture2D> vulkanTexture = texture;
		WeakRef<VulkanStagingBuffer> vulkanBuffer = buffer;

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;  // Tightly packed
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { (int)x, (int)y, 0 };
		region.imageExtent = { width, height, 1};

		// Copy the image to the staging buffer
		vkCmdCopyImageToBuffer(mCommandBuffer, vulkanTexture->GetVulkanImage().Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			vulkanBuffer->GetBuffer(), 1, &region);
	}

	void VulkanCommandBuffer::SetPushConstants(WeakRef<GraphicsPipeline> shader, ShaderStage stage, const void* data, uint32_t size)
	{
		WeakRef<VulkanGraphicsPipeline> vulkanPipeline = shader;
		const auto& range = vulkanPipeline->GetPushConstant(GetShaderStage(stage));
		if (size > range.Size)
		{
			size = range.Size;
			SPDLOG_WARN("Attempting to push a constant that is to large for shader, stage: {}", (uint32_t)stage);
		}
		vkCmdPushConstants(mCommandBuffer, vulkanPipeline->GetPipelineLayout(), GetShaderStage(stage), range.Offset, size, data);
	}

	void VulkanCommandBuffer::BindComputePipeline(WeakRef<ComputePipeline> shader, const std::vector<WeakRef<ShaderResourceGroup>>& groups)
	{
		WeakRef<VulkanComputePipeline> vkPipeline = shader;
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline->GetPipeline());

		std::vector<VkDescriptorSet> sets;
		for (auto set : groups)
		{
			WeakRef<VulkanDescriptorSet> vulkanSet = set;
			sets.push_back(vulkanSet->GetDescriptorSet());
		}

		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline->GetPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
	}

	void VulkanCommandBuffer::SetPushConstants(WeakRef<ComputePipeline> shader, void* data, uint32_t size)
	{
		WeakRef<VulkanComputePipeline> vkPipeline = shader;
		vkCmdPushConstants(mCommandBuffer, vkPipeline->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, size, data);
	}

	void VulkanCommandBuffer::Execute(uint32_t workGroupsX, uint32_t workGroupsY, uint32_t workGroupsZ)
	{
		vkCmdDispatch(mCommandBuffer, workGroupsX, workGroupsY, workGroupsZ);
	}

	void VulkanCommandBuffer::BindMesh(WeakRef<Mesh> mesh)
	{
		VkDeviceSize offsets = 0;

		WeakRef<VulkanVertexBuffer> vulkanVertexBuffer = mesh->GetVertexBuffer();
		WeakRef<VulkanIndexBuffer> vulkanIndexBuffer = mesh->GetIndexBuffer();
		IndexType indexType = mesh->GetIndexBuffer()->GetIndexType();

		VkBuffer vertexBuffer = vulkanVertexBuffer->GetBuffer();
		VkBuffer indexBuffer = vulkanIndexBuffer->GetBuffer();

		vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &vertexBuffer, &offsets);
		vkCmdBindIndexBuffer(mCommandBuffer, indexBuffer, 0, GetIndexFormat(indexType));
	}

	void VulkanCommandBuffer::DrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount)
	{
		vkCmdDrawIndexed(mCommandBuffer, mesh->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
	}

	void VulkanCommandBuffer::BindAndDrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount)
	{
		BindMesh(mesh);
		DrawMesh(mesh, instanceCount);
	}
}