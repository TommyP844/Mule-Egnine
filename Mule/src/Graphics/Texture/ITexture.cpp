
#include "Graphics/Texture/ITexture.h"
#include "Graphics//Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

#include <set>

namespace Mule

{
	ITexture::ITexture(WeakRef<GraphicsContext> context)
		:
		mContext(context),
		mDevice(context->GetDevice())
	{
	}

	void ITexture::Initialize(void* data, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t mips, TextureFormat format, TextureFlags flags)
	{
		// TODO: deduce image type
		VkImageType imageType;
		VkImageViewType viewType;

		if (width > 1 && height == 1 && depth == 1)
		{
			imageType = VK_IMAGE_TYPE_1D;
			viewType = VK_IMAGE_VIEW_TYPE_1D;
			if (layers > 1)
				viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		}
		else if (width > 1 && height > 1 && depth == 1)
		{
			imageType = VK_IMAGE_TYPE_2D;
			viewType = VK_IMAGE_VIEW_TYPE_2D;
			if (layers > 1)
				viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
		else if (width > 1 && height > 1 && depth > 1)
		{
			imageType = VK_IMAGE_TYPE_3D;
			viewType = VK_IMAGE_VIEW_TYPE_3D;
			if (layers > 1)
			{
				SPDLOG_WARN("3D Textures connot have layers, defaulting to 1");
				layers = 1;
			}
		}

		mWidth = width;
		mHeight = height;
		mDepth = depth;
		mLayers = layers;
		mMips = mips;
		mFormat = format;

		VkImageCreateInfo info{};

		std::set<uint32_t> queueFamilyIndicexSet = {
			mContext->GetGraphicsQueue()->GetQueueFamilyIndex(),
			mContext->GetComputeQueue()->GetQueueFamilyIndex(),
			mContext->GetTransferQueue()->GetQueueFamilyIndex()
		};

		std::vector<uint32_t> queueFamilyIndices;
		std::copy(queueFamilyIndicexSet.begin(), queueFamilyIndicexSet.end(), std::back_inserter(queueFamilyIndices));

		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags;
		info.imageType = imageType;
		info.format = (VkFormat)format;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = depth;
		info.mipLevels = mips;
		info.arrayLayers = layers;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = queueFamilyIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = queueFamilyIndices.size();
		info.pQueueFamilyIndices = queueFamilyIndices.data();
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(mDevice, &info, nullptr, &mVulkanImage.Image);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to create image");
			return;
		}

		VkMemoryRequirements requierments;
		vkGetImageMemoryRequirements(mDevice, mVulkanImage.Image, &requierments);

		uint32_t memoryTypeIndex = mContext->GetMemoryTypeIndex(requierments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = requierments.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		allocInfo.pNext = nullptr;

		result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &mVulkanImage.Memory);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to allocate device memory");
		}

		result = vkBindImageMemory(mDevice, mVulkanImage.Image, mVulkanImage.Memory, 0);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to bind image memory");
			return;
		}

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = mVulkanImage.Image;
		viewCreateInfo.viewType = viewType;
		viewCreateInfo.format = (VkFormat)format; // Provided during swapchain creation
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = mips;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = layers;

		result = vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &mVulkanImage.ImageView);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create image view");
		}

		VkCommandBuffer commandBuffer = mContext->CreateSingleTimeCmdBuffer();

		// TODO: create staging buffer object
		// TODO: use command buffer object here
		// TODO: create proper layout transition system
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
		if (data != nullptr)
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = mVulkanImage.Image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);

			size_t size = width * height * depth * layers * GetFormatSize(mFormat);

			VkBufferCreateInfo bufferCreateInfo = {};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = size;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			
			vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &stagingBuffer);

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(mDevice, stagingBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = mContext->GetMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			vkAllocateMemory(mDevice, &allocInfo, nullptr, &stagingBufferMemory);
			vkBindBufferMemory(mDevice, stagingBuffer, stagingBufferMemory, 0);

			void* ptr;
			vkMapMemory(mDevice, stagingBufferMemory, 0, size, 0, &ptr);
			memcpy(ptr, data, size);
			vkUnmapMemory(mDevice, stagingBufferMemory);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = mLayers;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, mDepth };

			vkCmdCopyBufferToImage(
				commandBuffer,
				stagingBuffer,
				mVulkanImage.Image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region);
		}


		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;  // Current layout
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;  // Desired layout
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = mVulkanImage.Image;  // The Vulkan image you're transitioning
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // For color images
		barrier.subresourceRange.baseMipLevel = 0;  // First mip level
		barrier.subresourceRange.levelCount = 1;  // Number of mip levels
		barrier.subresourceRange.baseArrayLayer = 0;  // First array layer
		barrier.subresourceRange.layerCount = 1;  // Number of array layers

		// Define the source and destination access masks
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;  // Previous access
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;  // Next access

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,                                    
			0, nullptr,                           
			0, nullptr,                           
			1, &barrier
		);

		mContext->SubmitSingleTimeCmdBuffer(commandBuffer);
		mContext->WaitForSingleTimeCommands();
		vkFreeCommandBuffers(mDevice, mContext->GetSingleTimeCommandPool(), 1, &commandBuffer);

		if (stagingBufferMemory)
		{
			vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
			vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		}
	}
}