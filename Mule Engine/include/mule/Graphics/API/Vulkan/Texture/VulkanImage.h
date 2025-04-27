#pragma once

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	struct VulkanImage
	{
		VulkanImage()
			:
			Image(VK_NULL_HANDLE),
			ImageView(VK_NULL_HANDLE),
			Layout(VK_IMAGE_LAYOUT_UNDEFINED)
		{ }

		VkImage Image = VK_NULL_HANDLE;
		VkDeviceMemory Memory = VK_NULL_HANDLE;

		
		VkImageView ImageView = VK_NULL_HANDLE;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
}