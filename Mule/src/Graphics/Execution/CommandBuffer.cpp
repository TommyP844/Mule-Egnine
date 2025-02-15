#include "Graphics/Execution/CommandBuffer.h"

#include "Graphics/Execution/CommandPool.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
		:
		mCommandBuffer(commandBuffer),
		mDevice(device),
		mCommandPool(commandPool)
	{
	}

	//TODO: remove vkDeviceWaitIdle and use a fence
	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mCommandBuffer);
	}

	void CommandBuffer::Reset()
	{
		vkResetCommandBuffer(mCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void CommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo beginInfo{};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;
		beginInfo.pNext = nullptr;

		vkBeginCommandBuffer(mCommandBuffer, &beginInfo);
	}

	void CommandBuffer::End()
	{
		vkEndCommandBuffer(mCommandBuffer);
	}

	void CommandBuffer::TransitionSwapchainFrameBufferForRendering(Ref<SwapchainFrameBuffer> fb)
	{
		// Color Attachment
		{
			VulkanImage& imageData = fb->GetColorImage();

			auto oldLayout = imageData.Layout;
			auto newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			imageData.Layout = newLayout;

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = imageData.Image;

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else {
				SPDLOG_WARN("Invalid image transition");
			}


			vkCmdPipelineBarrier(
				mCommandBuffer,
				sourceStage, destinationStage,
				0,                 // Dependency flags
				0, nullptr,        // Memory barriers
				0, nullptr,        // Buffer memory barriers
				1, &barrier        // Image memory barriers
			);
		}

		// Depth Attachment
		{
			VulkanImage& imageData = fb->GetDepthImage();

			auto oldLayout = imageData.Layout;
			auto newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			imageData.Layout = newLayout;

			if (oldLayout == newLayout)
				return;

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = imageData.Image;

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0; // No access as it's undefined
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Assume minimal dependency
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else {
				SPDLOG_WARN("Invalid image transition");
			}



			vkCmdPipelineBarrier(
				mCommandBuffer,
				sourceStage, destinationStage,
				0,                 // Dependency flags
				0, nullptr,        // Memory barriers
				0, nullptr,        // Buffer memory barriers
				1, &barrier        // Image memory barriers
			);
		}
	}

	void CommandBuffer::TransitionSwapchainFrameBufferForPresent(Ref<SwapchainFrameBuffer> fb)
	{
		// Color Attachment
		{
			VulkanImage& imageData = fb->GetColorImage();

			auto oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			auto newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			imageData.Layout = newLayout;

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = imageData.Image;

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			// Transition from color attachment optimal to shader read only optimal first
			if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = 0; // No specific access required for present
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			}
			else {
				SPDLOG_ERROR("Invalid image transition");
			}

			vkCmdPipelineBarrier(
				mCommandBuffer,
				sourceStage, destinationStage,
				0,                 // Dependency flags
				0, nullptr,        // Memory barriers
				0, nullptr,        // Buffer memory barriers
				1, &barrier        // Image memory barriers
			);

		}

		// Depth Attachment
		{
			VulkanImage& imageData = fb->GetDepthImage();

			auto oldLayout = imageData.Layout;
			auto newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			imageData.Layout = newLayout;

			if (oldLayout == newLayout)
				return;

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = imageData.Image;

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0; // No access as it's undefined
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Assume minimal dependency
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
				// The depth attachment optimal layout indicates the image was used as a depth/stencil attachment.
				barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				// Transitioning to PRESENT_SRC_KHR means the image will be presented, so no specific destination access is required.
				barrier.dstAccessMask = 0;

				// The source stage should correspond to the depth/stencil attachment writes.
				sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

				// The destination stage for presenting to the screen doesn't require a specific stage.
				destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {

				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else {
				SPDLOG_WARN("Invalid image transition");
			}



			vkCmdPipelineBarrier(
				mCommandBuffer,
				sourceStage, destinationStage,
				0,                 // Dependency flags
				0, nullptr,        // Memory barriers
				0, nullptr,        // Buffer memory barriers
				1, &barrier        // Image memory barriers
			);
		}
	}

	void CommandBuffer::BeginRenderPass(Ref<SwapchainFrameBuffer> framebuffer)
	{
		VkRenderPassBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = framebuffer->GetRenderPass();
		info.framebuffer = framebuffer->GetHandle();
		auto clearValues = framebuffer->GetClearValues();
		info.clearValueCount = clearValues.size();
		info.pClearValues = clearValues.data();

		VkRect2D rect{};
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = framebuffer->GetWidth();
		rect.extent.height = framebuffer->GetHeight();
		info.renderArea = rect;
		
		info.pNext = nullptr;
		vkCmdBeginRenderPass(mCommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = framebuffer->GetWidth();
		viewport.height = framebuffer->GetHeight();
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);

		vkCmdSetScissor(mCommandBuffer, 0, 1, &rect);
	}

	void CommandBuffer::BeginRenderPass(Ref<FrameBuffer> framebuffer, Ref<RenderPass> renderPass)
	{
		VkRenderPassBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = renderPass->GetHandle();
		info.framebuffer = framebuffer->GetHandle();
		auto clearValues = framebuffer->GetClearValues();
		info.clearValueCount = clearValues.size();
		info.pClearValues = clearValues.data();

		VkRect2D rect{};
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = framebuffer->GetWidth();
		rect.extent.height = framebuffer->GetHeight();
		info.renderArea = rect;

		info.pNext = nullptr;
		vkCmdBeginRenderPass(mCommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = framebuffer->GetHeight();
		viewport.width = framebuffer->GetWidth();
		viewport.height = -framebuffer->GetHeight();
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);

		vkCmdSetScissor(mCommandBuffer, 0, 1, &rect);
	}
	
	void CommandBuffer::NextPass()
	{
		vkCmdNextSubpass(mCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(mCommandBuffer);
	}

	void CommandBuffer::TranistionImageLayout(WeakRef<ITexture> texture, ImageLayout newLayout)
	{
		VkImageLayout oldLayout = texture->GetVulkanImage().Layout;
		VkImageLayout newVkLayout = (VkImageLayout)newLayout;

		if (oldLayout == newVkLayout)
			return;

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		VkImageAspectFlags imageAspect = texture->IsDepthTexture() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		texture->SetImageLayout(newVkLayout);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newVkLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = texture->GetVulkanImage().Image;
		barrier.subresourceRange.aspectMask = imageAspect;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = texture->GetMipCount();
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = texture->GetLayerCount();
		

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
	}

	void CommandBuffer::CopyTexture(WeakRef<ITexture> src, WeakRef<ITexture> dst, const TextureCopyInfo& copyInfo) const
	{
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
		
		vkCmdCopyImage(mCommandBuffer, src->GetImage(), src->GetVulkanImage().Layout, dst->GetImage(), dst->GetVulkanImage().Layout, 1, &region);
	}

	void CommandBuffer::BindGraphicsPipeline(WeakRef<GraphicsShader> shader)
	{
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipeline());
	}

	void CommandBuffer::BindComputePipeline(WeakRef<ComputeShader> shader)
	{
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->GetPipeline());
	}

	void CommandBuffer::SetPushConstants(WeakRef<GraphicsShader> shader, ShaderStage stage, void* data, uint32_t size)
	{
		const auto& range = shader->GetPushConstantRange(stage);
		if (size > range.second)
		{
			size = range.second;
			SPDLOG_WARN("Attempting to push a constant that is to large for shader: {}, and stage: {}", shader->Name(), (uint32_t)stage);
		}
		vkCmdPushConstants(mCommandBuffer, shader->GetPipelineLayout(), (VkShaderStageFlags)stage, range.first, size, data);
	}

	void CommandBuffer::BindGraphicsDescriptorSet(WeakRef<GraphicsShader> shader, const std::vector<WeakRef<DescriptorSet>>& descriptorSets)
	{
		std::vector<VkDescriptorSet> sets;
		for (auto set : descriptorSets)
		{
			sets.push_back(set->GetDescriptorSet());
		}
		
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
	}

	void CommandBuffer::BindComputeDescriptorSet(WeakRef<ComputeShader> shader, WeakRef<DescriptorSet> descriptorSet)
	{
		VkDescriptorSet descriptorSetPtr = descriptorSet->GetDescriptorSet();
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->GetPipelineLayout(), 0, 1, &descriptorSetPtr, 0, nullptr);
	}

	void CommandBuffer::Execute(uint32_t workGroupsX, uint32_t workGroupsY, uint32_t workGroupsZ)
	{
		vkCmdDispatch(mCommandBuffer, workGroupsX, workGroupsY, workGroupsZ);
	}

	void CommandBuffer::BindMesh(WeakRef<Mesh> mesh)
	{
		VkDeviceSize offsets = 0;

		VkBuffer vertexBuffer = mesh->GetVertexBuffer()->GetBuffer();
		VkBuffer indexBuffer = mesh->GetIndexBuffer()->GetBuffer();
		IndexBufferType indexType = mesh->GetIndexBuffer()->GetBufferType();

		vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &vertexBuffer, &offsets);
		vkCmdBindIndexBuffer(mCommandBuffer, indexBuffer, 0, (VkIndexType)indexType);
	}

	void CommandBuffer::DrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount)
	{
		vkCmdDrawIndexed(mCommandBuffer, mesh->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
	}

	void CommandBuffer::BindAndDrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount)
	{
		BindMesh(mesh);
		DrawMesh(mesh, instanceCount);
	}
}