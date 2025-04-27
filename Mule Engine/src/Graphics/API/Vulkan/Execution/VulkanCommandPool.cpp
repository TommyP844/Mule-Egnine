#include "Graphics/API/Vulkan/Execution/VulkanCommandPool.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include "Graphics/API/Vulkan/Execution/VulkanCommandBuffer.h"

namespace Mule::Vulkan
{
	VulkanCommandPool::VulkanCommandPool()
		:
		mCommandPool(VK_NULL_HANDLE)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		uint32_t queueFamilyIndex = context.GetQueueFamilyIndex();

		VkCommandPoolCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queueFamilyIndex
		};		

		vkCreateCommandPool(device, &createInfo, nullptr, &mCommandPool);
	}

	VulkanCommandPool::~VulkanCommandPool()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkDestroyCommandPool(device, mCommandPool, nullptr);
	}

	void VulkanCommandPool::Reset()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkResetCommandPool(device, mCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	Ref<CommandBuffer> VulkanCommandPool::CreateCommandBuffer()
	{
		return MakeRef<VulkanCommandBuffer>(mCommandPool);
	}
}
