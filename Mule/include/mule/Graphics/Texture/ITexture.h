#pragma once

#include "WeakRef.h"
#include "Graphics/RenderTypes.h"

namespace Mule
{
	class GraphicsContext;

	class ITexture
	{
	public:
		ITexture(WeakRef<GraphicsContext> context);
		virtual ~ITexture()
		{
			vkDeviceWaitIdle(mDevice);
			vkFreeMemory(mDevice, mVulkanImage.Memory, nullptr);
			vkDestroyImageView(mDevice, mVulkanImage.ImageView, nullptr);
			vkDestroyImage(mDevice, mVulkanImage.Image, nullptr);
		}

		VkImage GetImage() const { return mVulkanImage.Image; }
		VkImageView GetImageView() const { return mVulkanImage.ImageView; }

		TextureFormat GetFormat() const { return mFormat; }
		uint32_t GetLayerCount() const { return mLayers; }
		uint32_t GetMipCount() const { return mMips; }
		uint32_t GetWidth() const { return mWidth; }
		uint32_t GetHeight() const { return mHeight; }
		uint32_t GetDepth() const { return mDepth; }

	protected:
		void Initialize(void* data, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t mips, TextureFormat format, TextureFlags flags);

		WeakRef<GraphicsContext> mContext;
		VkDevice mDevice;
		VulkanImage mVulkanImage;
		
		uint32_t mWidth, mHeight, mDepth, mMips, mLayers;
		TextureFormat mFormat;
	};
}