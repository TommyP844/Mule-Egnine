#include "Graphics/API/Vulkan/VulkanCommandPool.h"

#include "Graphics/API/Vulkan/VulkanCommandBuffer.h"

namespace Mule
{
	VulkanCommandPool::~VulkanCommandPool()
	{
		vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	}

	void VulkanCommandPool::Reset()
	{
		vkResetCommandPool(mDevice, mCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	Ref<VulkanCommandBuffer> Mule::VulkanCommandPool::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = mCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.pNext = nullptr;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);
		return MakeRef<VulkanCommandBuffer>(mDevice, mCommandPool, commandBuffer);
	}
}