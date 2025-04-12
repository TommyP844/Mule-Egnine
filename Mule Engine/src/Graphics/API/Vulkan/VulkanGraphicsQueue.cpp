#include "Graphics/API/Vulkan/VulkanGraphicsQueue.h"

#include "Graphics/API/Vulkan/VulkanCommandBuffer.h"

namespace Mule
{
	void VulkanGraphicsQueue::Submit(Ref<VulkanCommandBuffer> commandbuffer, const std::vector<WeakRef<VulkanSemaphore>>& waitSemaphores, const std::vector<WeakRef<VulkanSemaphore>>& signalSemaphores, Ref<VulkanFence> fence)
	{
		VkCommandBuffer buffer = commandbuffer->GetHandle();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer;

		std::vector<VkPipelineStageFlags> waitStages;
		std::vector<VkSemaphore> signalSemaphoresVK;
		for (auto& signal : signalSemaphores)
		{
			waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			signalSemaphoresVK.push_back(signal->GetHandle());
		}

		submitInfo.signalSemaphoreCount = signalSemaphoresVK.size();
		submitInfo.pSignalSemaphores = signalSemaphoresVK.data();

		std::vector<VkSemaphore> waitSemaphoresVK;
		for (auto& signal : waitSemaphores)
		{
			waitSemaphoresVK.push_back(signal->GetHandle());
			waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}

		submitInfo.waitSemaphoreCount = waitSemaphoresVK.size();
		submitInfo.pWaitSemaphores = waitSemaphoresVK.data();
		submitInfo.pWaitDstStageMask = waitStages.data();

		submitInfo.pNext = nullptr;

		std::lock_guard<std::mutex> lock(mMutex);
		VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, (fence ? fence->GetHandle() : VK_NULL_HANDLE));
	}

	void VulkanGraphicsQueue::Present(uint32_t imageIndex, VkSwapchainKHR swapchain, const std::vector<WeakRef<VulkanSemaphore>>& waitSemaphores, WeakRef<VulkanFence> fence)
	{
		std::vector<VkSemaphore> waitSemaphoresVK;
		for (const auto& semaphore : waitSemaphores)
		{
			waitSemaphoresVK.emplace_back(semaphore->GetHandle());
		}

		uint32_t indices[] = { imageIndex };
		VkSwapchainKHR swapchains[] = { swapchain };

		VkResult submitResult{};
		
		VkPresentInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.swapchainCount = 1;
		info.pSwapchains = swapchains;
		info.waitSemaphoreCount = waitSemaphoresVK.size();
		info.pWaitSemaphores = waitSemaphoresVK.data();
		info.pImageIndices = indices;
		info.pResults = &submitResult;
		info.pNext = nullptr;

		std::lock_guard<std::mutex> lock(mMutex);
		VkResult result = vkQueuePresentKHR(mQueue, &info);
	}

	Ref<VulkanCommandPool> VulkanGraphicsQueue::CreateCommandPool()
	{
		VkCommandPoolCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = mQueueFamilyIndex;
		createInfo.pNext = nullptr;

		VkCommandPool commandPool;
		std::lock_guard<std::mutex> lock(mMutex);
		vkCreateCommandPool(mDevice, &createInfo, nullptr, &commandPool);
		return MakeRef<VulkanCommandPool>(mDevice, commandPool);
	}
}