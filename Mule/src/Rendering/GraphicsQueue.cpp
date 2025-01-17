#include "Rendering/GraphicsQueue.h"

#include "Rendering/CommandBuffer.h"

namespace Mule
{
	void GraphicsQueue::Submit(Ref<CommandBuffer> commandbuffer, const std::vector<Ref<Semaphore>>& waitGPUFences, const std::vector<Ref<Semaphore>>& signalGPUFences, Ref<Fence> cpuFence)
	{
		VkCommandBuffer buffer = commandbuffer->GetHandle();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer;

		std::vector<VkSemaphore> signalSemaphores;
		for (auto& signal : signalGPUFences)
		{
			signalSemaphores.push_back(signal->GetHandle());
		}

		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores = signalSemaphores.data();

		std::vector<VkSemaphore> waitSemaphores;
		for (auto& signal : waitGPUFences)
		{
			waitSemaphores.push_back(signal->GetHandle());
		}

		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = nullptr;


		submitInfo.pNext = nullptr;

		VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, (cpuFence ? cpuFence->GetHandle() : VK_NULL_HANDLE));
	}

	Ref<CommandPool> GraphicsQueue::CreateCommandPool()
	{
		VkCommandPoolCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = mQueueFamilyIndex;
		createInfo.pNext = nullptr;

		VkCommandPool commandPool;
		vkCreateCommandPool(mDevice, &createInfo, nullptr, &commandPool);
		return MakeRef<CommandPool>(mDevice, commandPool);
	}
}