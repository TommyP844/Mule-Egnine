#include "Rendering/GraphicsQueue.h"

#include "Rendering/CommandBuffer.h"

namespace Mule
{
	void GraphicsQueue::Submit(Ref<CommandBuffer> commandbuffer)
	{
		VkCommandBuffer buffer = commandbuffer->GetHandle();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer;

		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;


		submitInfo.pNext = nullptr;

		vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
	}

	Ref<CommandPool> GraphicsQueue::CreateCommandPool()
	{
		VkCommandPoolCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = 0;
		createInfo.queueFamilyIndex = mQueueFamilyIndex;
		createInfo.pNext = nullptr;

		VkCommandPool commandPool;
		vkCreateCommandPool(mDevice, &createInfo, nullptr, &commandPool);
		return MakeRef<CommandPool>(mDevice, commandPool);
	}
}