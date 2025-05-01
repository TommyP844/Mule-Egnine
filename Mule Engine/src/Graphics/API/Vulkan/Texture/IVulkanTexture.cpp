
#include "Graphics/API/Vulkan/Texture/IVulkanTexture.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

namespace Mule::Vulkan
{
	bool IVulkanTexture::CreateImage(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		uint32_t queueFamilyIndex = context.GetQueueFamilyIndex();

		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.imageType = type;
		info.format = format;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = depth;
		info.mipLevels = mipLevels;
		info.arrayLayers = arrayLayers;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usageFlags;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 1;
		info.pQueueFamilyIndices = &queueFamilyIndex;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(device, &info, nullptr, &mVulkanImage.Image);
		if (result != VK_SUCCESS)
			return false;
		
		return true;
	}

	bool IVulkanTexture::CreateCubeImage(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		uint32_t queueFamilyIndex = context.GetQueueFamilyIndex();

		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		info.imageType = type;
		info.format = format;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = depth;
		info.mipLevels = mipLevels;
		info.arrayLayers = arrayLayers;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usageFlags;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 1;
		info.pQueueFamilyIndices = &queueFamilyIndex;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(device, &info, nullptr, &mVulkanImage.Image);
		if (result != VK_SUCCESS)
			return false;

		return true;
	}

	bool IVulkanTexture::AllocateMemory()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		VkMemoryRequirements requierments;
		vkGetImageMemoryRequirements(device, mVulkanImage.Image, &requierments);

		uint32_t memoryTypeIndex = context.GetMemoryTypeIndex(requierments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = requierments.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		allocInfo.pNext = nullptr;

		VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &mVulkanImage.Memory);
		if (result != VK_SUCCESS)
			return false;

		result = vkBindImageMemory(device, mVulkanImage.Image, mVulkanImage.Memory, 0);
		if (result != VK_SUCCESS)
			return false;

		return true;
	}

	WeakRef<VulkanTextureView> IVulkanTexture::GetTextureView(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return mTextureViews[arrayLayer][mipLevel];
	}

	bool IVulkanTexture::Init(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags, VkImageAspectFlags aspect, VkImageViewType viewType)
	{
		mWidth = width;
		mHeight = height;
		mDepth = depth;

		mMipLevels = mipLevels;
		mArrayLayers = arrayLayers;

		mImageAspect = aspect;
		bool success = CreateImage(type, format, width, height, depth, mipLevels, arrayLayers, usageFlags);
		success |= AllocateMemory();
		success |= CreateView(mVulkanImage.ImageView, viewType, format, aspect, 0, mipLevels, 0, arrayLayers);

		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		// TODO: eventually make these generated on the fly
		mTextureViews.resize(arrayLayers);

		for (uint32_t layer = 0; layer < arrayLayers; layer++)
		{
			mTextureViews[layer].resize(mipLevels);
			for (uint32_t level = 0; level < mipLevels; level++)
			{
				VkImageViewCreateInfo viewInfo{};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.pNext = nullptr;
				viewInfo.flags = 0;
				viewInfo.image = mVulkanImage.Image;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = format;
				viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.subresourceRange.aspectMask = aspect;
				viewInfo.subresourceRange.baseArrayLayer = layer;
				viewInfo.subresourceRange.layerCount = 1;
				viewInfo.subresourceRange.baseMipLevel = level;
				viewInfo.subresourceRange.levelCount = 1;

				VkImageView view;
				vkCreateImageView(device, &viewInfo, nullptr, &view);

				mTextureViews[layer][level] = MakeRef<VulkanTextureView>(view);
			}
		}

		return success;
	}

	bool IVulkanTexture::InitCube(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags, VkImageAspectFlags aspect, VkImageViewType viewType)
	{
		mWidth = width;
		mHeight = height;
		mDepth = depth;

		mMipLevels = mipLevels;
		mArrayLayers = arrayLayers;

		mMipViews.resize(mipLevels);

		mIsDepthTexture = (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) ? true : false;

		mImageAspect = aspect;
		bool success = CreateCubeImage(type, format, width, height, depth, mipLevels, arrayLayers, usageFlags);
		success |= AllocateMemory();
		success |= CreateView(mVulkanImage.ImageView, viewType, format, aspect, 0, mipLevels, 0, arrayLayers);

		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		mTextureViews.resize(arrayLayers);

		for (uint32_t layer = 0; layer < arrayLayers; layer++)
		{
			mTextureViews[layer].resize(mipLevels);
			for (uint32_t level = 0; level < mipLevels; level++)
			{
				VkImageViewCreateInfo viewInfo{};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.pNext = nullptr;
				viewInfo.flags = 0;
				viewInfo.image = mVulkanImage.Image;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = format;
				viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewInfo.subresourceRange.aspectMask = aspect;
				viewInfo.subresourceRange.baseArrayLayer = layer;
				viewInfo.subresourceRange.layerCount = 1;
				viewInfo.subresourceRange.baseMipLevel = level;
				viewInfo.subresourceRange.levelCount = 1;

				VkImageView view;
				vkCreateImageView(device, &viewInfo, nullptr, &view);

				mTextureViews[layer][level] = MakeRef<VulkanTextureView>(view);
			}
		}

		return success;
	}

	bool IVulkanTexture::CreateView(VkImageView& view, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspect, uint32_t baseMipLevel, uint32_t mipCount, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = mVulkanImage.Image;
		viewCreateInfo.viewType = viewType;
		viewCreateInfo.format = format;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = aspect;
		viewCreateInfo.subresourceRange.baseMipLevel = baseMipLevel;
		viewCreateInfo.subresourceRange.levelCount = mipCount;
		viewCreateInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
		viewCreateInfo.subresourceRange.layerCount = layerCount;

		VkResult result = vkCreateImageView(device, &viewCreateInfo, nullptr, &mVulkanImage.ImageView);
		if (result != VK_SUCCESS)
			return false;

		return true;
	}

	IVulkanTexture::~IVulkanTexture()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkDestroyImageView(device, mVulkanImage.ImageView, nullptr);
		vkDestroyImage(device, mVulkanImage.Image, nullptr);
		vkFreeMemory(device, mVulkanImage.Memory, nullptr);
	}

	void IVulkanTexture::SetImageLayout(VkImageLayout imageLayout)
	{
		mVulkanImage.Layout = imageLayout;
	}

	VkImageLayout IVulkanTexture::GetImageLayout() const
	{
		return mVulkanImage.Layout;
	}

	const VulkanImage& IVulkanTexture::GetVulkanImage() const
	{
		return mVulkanImage;
	}

	VkImageAspectFlags IVulkanTexture::GetImageAspect() const
	{
		return mImageAspect;
	}
}