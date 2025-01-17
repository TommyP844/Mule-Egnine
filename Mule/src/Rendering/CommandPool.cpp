#include "Rendering/CommandPool.h"

#include "Rendering/CommandBuffer.h"

namespace Mule
{
	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	}

	void CommandPool::Reset()
	{
		vkResetCommandPool(mDevice, mCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	Ref<CommandBuffer> Mule::CommandPool::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = mCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.pNext = nullptr;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);
		return MakeRef<CommandBuffer>(mDevice, mCommandPool, commandBuffer);
	}
}