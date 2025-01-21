#pragma once

#include "Graphics/RenderTypes.h"

namespace Mule
{
	class ITexture
	{
	public:
		ITexture(VkDevice device) : mDevice(device) {}
		virtual ~ITexture()
		{
			vkDeviceWaitIdle(mDevice);
			vkFreeMemory(mDevice, mVulkanImage.Memory, nullptr);
			vkDestroyImageView(mDevice, mVulkanImage.ImageView, nullptr);
			vkDestroyImage(mDevice, mVulkanImage.Image, nullptr);
		}

		VkSampler GetSampler() const { return mSampler; }
		VkImage GetImage() const { return mVulkanImage.Image; }
		VkImageView GetImageView() const { return mVulkanImage.ImageView; }

	protected:
		VkDevice mDevice;
		VulkanImage mVulkanImage;
		VkSampler mSampler;
	};
}