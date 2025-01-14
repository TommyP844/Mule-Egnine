#pragma once

#include "CommandPool.h"


#include <vulkan/vulkan.h>

namespace Mule
{
	class GraphicsQueue
	{
	public:
		GraphicsQueue(VkDevice device, VkQueue queue, uint32_t familyIndex) : mDevice(device), mQueue(queue), mQueueFamilyIndex(familyIndex) {}

		VkQueue GetHandle() const { return mQueue; }
		uint32_t GetQueueFamilyIndex() const { return mQueueFamilyIndex; }

		void Submit(Ref<CommandBuffer> commandbuffer);

		Ref<CommandPool> CreateCommandPool();
	private:
		VkDevice mDevice;
		VkQueue mQueue;
		uint32_t mQueueFamilyIndex;
	};
}