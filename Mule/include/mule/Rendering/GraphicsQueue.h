#pragma once

#include "CommandPool.h"
#include "Fence.h"
#include "Semaphore.h"


#include <vulkan/vulkan.h>

namespace Mule
{
	class GraphicsQueue
	{
	public:
		GraphicsQueue(VkDevice device, VkQueue queue, uint32_t familyIndex) : mDevice(device), mQueue(queue), mQueueFamilyIndex(familyIndex) {}

		VkQueue GetHandle() const { return mQueue; }
		uint32_t GetQueueFamilyIndex() const { return mQueueFamilyIndex; }

		void Submit(Ref<CommandBuffer> commandbuffer, const std::vector<Ref<Semaphore>>& waitGPUFences = {}, const std::vector<Ref<Semaphore>>& signalGPUFences = {}, Ref<Fence> cpuFence = nullptr);

		Ref<CommandPool> CreateCommandPool();
	private:
		VkDevice mDevice;
		VkQueue mQueue;
		uint32_t mQueueFamilyIndex;
	};
}