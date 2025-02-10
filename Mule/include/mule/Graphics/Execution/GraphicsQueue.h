#pragma once

#include "CommandPool.h"
#include "Fence.h"
#include "Semaphore.h"
#include "WeakRef.h"

#include <mutex>

#include <vulkan/vulkan.h>

namespace Mule
{
	class GraphicsQueue
	{
	public:
		GraphicsQueue(VkDevice device, VkQueue queue, uint32_t familyIndex) : mDevice(device), mQueue(queue), mQueueFamilyIndex(familyIndex) {}

		VkQueue GetHandle() const { return mQueue; }
		uint32_t GetQueueFamilyIndex() const { return mQueueFamilyIndex; }

		void Submit(Ref<CommandBuffer> commandbuffer, const std::vector<Ref<Semaphore>>& waitSemaphores = {}, const std::vector<Ref<Semaphore>>& signalSemaphores = {}, Ref<Fence> fence = nullptr);
		void Present(uint32_t imageIndex, VkSwapchainKHR swapchain, const std::vector<WeakRef<Semaphore>>& waitSemaphores = {}, WeakRef<Fence> fence = nullptr);

		Ref<CommandPool> CreateCommandPool();
	private:
		VkDevice mDevice;
		VkQueue mQueue;
		uint32_t mQueueFamilyIndex;

		std::mutex mMutex;
	};
}