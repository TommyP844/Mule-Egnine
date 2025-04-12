#pragma once

#include <Ref.h>

#include <Volk/volk.h>

namespace Mule
{
	class VulkanCommandBuffer;

	class VulkanCommandPool
	{
	public:
		VulkanCommandPool(VkDevice device, VkCommandPool commandPool) : mDevice(device), mCommandPool(commandPool) {}
		~VulkanCommandPool();

		void Reset();
		Ref<VulkanCommandBuffer> CreateCommandBuffer();

	private:
		VkDevice mDevice;
		VkCommandPool mCommandPool;
	};
}