#pragma once

#include "Ref.h"
#include "WeakRef.h"
#include "VulkanImage.h"
#include "VulkanTextureView.h"

#include <vector>

namespace Mule::Vulkan
{
	class IVulkanTexture
	{
	public:
		IVulkanTexture() = default;
		virtual ~IVulkanTexture();
		void SetImageLayout(VkImageLayout imageLayout);
		VkImageLayout GetImageLayout() const;
		const VulkanImage& GetVulkanImage() const;
		VkImageAspectFlags GetImageAspect() const;

		uint32_t GetImageWidth() const { return mWidth; }
		uint32_t GetImageHeight() const { return mHeight; }
		uint32_t GetImageDepth() const { return mDepth; }

		uint32_t GetImageMipLevels() const { return mMipLevels; }
		uint32_t GetImageArrayLayers() const { return mArrayLayers; }

		bool GetIsDepthTexture() const { return mIsDepthTexture; }

		WeakRef<VulkanTextureView> GetTextureView(uint32_t mipLevel, uint32_t arrayLayer) const;

	protected:

		bool Init(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags, VkImageAspectFlags aspect, VkImageViewType viewType);
		bool InitCube(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags, VkImageAspectFlags aspect, VkImageViewType viewType);

		bool CreateView(VkImageView& view, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspect, uint32_t baseMipLevel, uint32_t mipCount, uint32_t baseArrayLayer, uint32_t layerCount);

		uint32_t mWidth, mHeight, mDepth;
		uint32_t mMipLevels, mArrayLayers;
		bool mIsDepthTexture;

		std::vector<Ref<VulkanTextureView>> mMipViews;

	private:
		bool CreateImage(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags);
		bool CreateCubeImage(VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usageFlags);
		bool AllocateMemory();
		VulkanImage mVulkanImage;
		VkImageAspectFlags mImageAspect;
		
		std::vector<std::vector<Ref<VulkanTextureView>>> mTextureViews;
		
	};
}