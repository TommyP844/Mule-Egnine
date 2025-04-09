#pragma once

#include <Ref.h>


#include <Volk/volk.h>

namespace Mule
{
	class CommandBuffer;

	class CommandPool
	{
	public:
		CommandPool(VkDevice device, VkCommandPool commandPool) : mDevice(device), mCommandPool(commandPool) {}
		~CommandPool();

		void Reset();
		Ref<CommandBuffer> CreateCommandBuffer();

	private:
		VkDevice mDevice;
		VkCommandPool mCommandPool;
	};
}