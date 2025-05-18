#include "Graphics/API/Vulkan/Syncronization/VulkanTimelineSemaphore.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	VulkanTimelineSemaphore::VulkanTimelineSemaphore()
	{
		VulkanContext& context = VulkanContext::Get();

		VkSemaphoreTypeCreateInfo timelineCreateInfo{};
		timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = 0;

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = &timelineCreateInfo;

		VkResult result = vkCreateSemaphore(context.GetDevice(), &semaphoreCreateInfo, nullptr, &mSemaphore);
	}

	VulkanTimelineSemaphore::~VulkanTimelineSemaphore()
	{
		VulkanContext& context = VulkanContext::Get();
		vkDestroySemaphore(context.GetDevice(), mSemaphore, nullptr);
	}

	uint64_t VulkanTimelineSemaphore::GetValue() const
	{
		VulkanContext& context = VulkanContext::Get();

		uint64_t value = 0;
		VkResult result = vkGetSemaphoreCounterValueKHR(context.GetDevice(), mSemaphore, &value);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to get Timeline Semaphore Value");
			return 0;
		}

		return value;
	}
}