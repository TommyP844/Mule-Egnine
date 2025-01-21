#include "Graphics/Framebuffer.h"

#include "Graphics/Context/GraphicsContext.h"

// Submodule
#include <spdlog/spdlog.h>

// STD
#include <set>

namespace Mule
{
	FrameBuffer::FrameBuffer(WeakRef<GraphicsContext> context, const FramebufferDescription& desc)
		:
		mContext(context),
		mDesc(desc)
	{
		Resize(mDesc.Width, mDesc.Height);
	}

	FrameBuffer::~FrameBuffer()
	{
		Invalidate();
	}

	void FrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		// TODO: destroy framebuffer if it exists

		mDesc.Width = width;
		mDesc.Height = height;

		std::vector<VkImageView> framebufferViews;

		for (auto attachment : mDesc.Attachments)
		{
			VulkanImage image;
			image.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			CreateImage(
				image, 
				mDesc.Width, 
				mDesc.Height, 
				mDesc.LayerCount, 
				1, 
				(VkFormat)attachment.Format, 
				(mDesc.LayerCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY),
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				false);

			mColorAttachments.push_back(image);
			framebufferViews.push_back(image.ImageView);
		}

		if (mDesc.DepthAttachment.Format != TextureFormat::NONE)
		{
			mDepthAttachment.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			CreateImage(
				mDepthAttachment,
				mDesc.Width,
				mDesc.Height,
				mDesc.LayerCount,
				1,
				(VkFormat)mDesc.DepthAttachment.Format,
				(mDesc.LayerCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY),
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				true);

			framebufferViews.push_back(mDepthAttachment.ImageView);
		}

		VkFramebufferCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = mDesc.Width;
		info.height = mDesc.Height;
		info.layers = mDesc.LayerCount;
		info.renderPass = mDesc.RenderPass->GetHandle();
		info.flags = 0;
		info.attachmentCount = framebufferViews.size();
		info.pAttachments = framebufferViews.data();
		info.pNext = nullptr;

		VkResult result = vkCreateFramebuffer(mContext->GetDevice(), &info, nullptr, &mFrameBuffer);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create framebuffer");
		}
	}

	void FrameBuffer::Invalidate()
	{
		for (auto& image : mColorAttachments)
		{
			vkDestroyImageView(mContext->GetDevice(), image.ImageView, nullptr);
			vkFreeMemory(mContext->GetDevice(), image.Memory, nullptr);
			vkDestroyImage(mContext->GetDevice(), image.Image, nullptr);
		}

		if (mDepthAttachment.Image != VK_NULL_HANDLE)
		{
			vkDestroyImageView(mContext->GetDevice(), mDepthAttachment.ImageView, nullptr);
			vkFreeMemory(mContext->GetDevice(), mDepthAttachment.Memory, nullptr);
			vkDestroyImage(mContext->GetDevice(), mDepthAttachment.Image, nullptr);
		}

		vkDestroyFramebuffer(mContext->GetDevice(), mFrameBuffer, nullptr);
	}

	bool FrameBuffer::CreateImage(VulkanImage& image, uint32_t width, uint32_t height, uint32_t layers, uint32_t mips, VkFormat format, VkImageViewType viewtype, VkImageUsageFlagBits usage, bool depthImage)
	{
		bool success = true;

		VkImageCreateInfo info{};

		std::set<uint32_t> queueFamilyIndicexSet = {
			mContext->GetGraphicsQueue()->GetQueueFamilyIndex(),
			mContext->GetTransferQueue()->GetQueueFamilyIndex(),
			mContext->GetComputeQueue()->GetQueueFamilyIndex()
		};

		std::vector<uint32_t> queueFamilyIndices;
		std::copy(queueFamilyIndicexSet.begin(), queueFamilyIndicexSet.end(), std::back_inserter(queueFamilyIndices));

		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = format;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;
		info.mipLevels = mips;
		info.arrayLayers = layers;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usage;
		info.sharingMode = queueFamilyIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = queueFamilyIndices.size();
		info.pQueueFamilyIndices = queueFamilyIndices.data();
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(mContext->GetDevice(), &info, nullptr, &image.Image);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to bind image memory");
			return false;
		}


		uint32_t size = width * height * layers * GetFormatSize((TextureFormat)format);

		VkMemoryRequirements requierments;
		vkGetImageMemoryRequirements(mContext->GetDevice(), image.Image, &requierments);

		auto memoryProperties = mContext->GetMemoryProperties();
		uint32_t memoryTypeIndex = 0;
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			// Check if the memory type is supported
			if ((requierments.memoryTypeBits & (1 << i)) &&
				(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
				memoryTypeIndex = i;
			}
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = requierments.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		allocInfo.pNext = nullptr;

		result = vkAllocateMemory(mContext->GetDevice(), &allocInfo, nullptr, &image.Memory);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to bind image memory");
			vkDestroyImage(mContext->GetDevice(), image.Image, nullptr);
			return false;
		}

		result = vkBindImageMemory(mContext->GetDevice(), image.Image, image.Memory, 0);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to bind image memory");
			vkDestroyImage(mContext->GetDevice(), image.Image, nullptr);
			vkFreeMemory(mContext->GetDevice(), image.Memory, nullptr);
			return false;
		}

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = image.Image;
		viewCreateInfo.viewType = viewtype;
		viewCreateInfo.format = format; // Provided during swapchain creation
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = depthImage ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = mips;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = layers;

		result = vkCreateImageView(mContext->GetDevice(), &viewCreateInfo, nullptr, &image.ImageView);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create image view");

			vkFreeMemory(mContext->GetDevice(), image.Memory, nullptr);
			vkDestroyImage(mContext->GetDevice(), image.Image, nullptr);

			return false;
		}

		return true;
	}
}