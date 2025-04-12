#pragma once

#include "VulkanCommandPool.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "WeakRef.h"

#include <mutex>

#include <Volk/volk.h>

namespace Mule
{
	class VulkanGraphicsQueue
	{
	public:
		VulkanGraphicsQueue(VkDevice device, VkQueue queue, uint32_t familyIndex) : mDevice(device), mQueue(queue), mQueueFamilyIndex(familyIndex) {}

		VkQueue GetHandle() const { return mQueue; }
		uint32_t GetQueueFamilyIndex() const { return mQueueFamilyIndex; }

		void Submit(Ref<VulkanCommandBuffer> commandbuffer, const std::vector<WeakRef<VulkanSemaphore>>& waitSemaphores = {}, const std::vector<WeakRef<VulkanSemaphore>>& signalSemaphores = {}, Ref<VulkanFence> fence = nullptr);
		void Present(uint32_t imageIndex, VkSwapchainKHR swapchain, const std::vector<WeakRef<VulkanSemaphore>>& waitSemaphores = {}, WeakRef<VulkanFence> fence = nullptr);

		Ref<VulkanCommandPool> CreateCommandPool();
	private:
		VkDevice mDevice;
		VkQueue mQueue;
		uint32_t mQueueFamilyIndex;

		std::mutex mMutex;
	};
}