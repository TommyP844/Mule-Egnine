#pragma once

#include <Ref.h>


#include <vulkan/vulkan.h>

namespace Mule
{
	class CommandBuffer;

	class CommandPool
	{
	public:
		CommandPool(VkDevice device, VkCommandPool commandPool) : mCommandPool(commandPool) {}
		~CommandPool() {}

		Ref<CommandBuffer> CreateCommandbuffer();

	private:
		VkDevice mDevice;
		VkCommandPool mCommandPool;
	};
}