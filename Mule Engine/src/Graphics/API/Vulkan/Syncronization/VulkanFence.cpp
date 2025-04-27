
#include "Graphics/API/Vulkan/Syncronization/VulkanFence.h"
#include "Graphics/API/Vulkan/VulkanContext.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	VulkanFence::VulkanFence()
		:
		mFence(VK_NULL_HANDLE)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		VkFenceCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		info.pNext = nullptr;

		vkCreateFence(device, &info, nullptr, &mFence);
	}
	
	VulkanFence::~VulkanFence()
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		vkDestroyFence(device, mFence, nullptr);
	}
	
	void VulkanFence::Reset()
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		vkResetFences(device, 1, &mFence);
	}

	void VulkanFence::Wait()
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		vkWaitForFences(device, 1, &mFence, VK_TRUE, UINT64_MAX);
	}
}