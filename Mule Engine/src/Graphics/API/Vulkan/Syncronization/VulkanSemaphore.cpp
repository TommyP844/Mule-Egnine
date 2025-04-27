
#include "Graphics/API/Vulkan/Syncronization/VulkanSemaphore.h"
#include "Graphics/API/Vulkan/VulkanContext.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	VulkanSemaphore::VulkanSemaphore()
		:
		mSemaphore(VK_NULL_HANDLE)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		VkSemaphoreCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;

		vkCreateSemaphore(device, &info, nullptr, &mSemaphore);
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		vkDestroySemaphore(device, mSemaphore, nullptr);
	}
}