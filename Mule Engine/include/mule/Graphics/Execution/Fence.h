#pragma once

#include <vulkan/vulkan.h>


namespace Mule
{
	class Fence
	{
	public:
		Fence(VkDevice device, VkFence fence) : mDevice(device), mFence(fence) {}
		~Fence()
		{
			vkDestroyFence(mDevice, mFence, nullptr);
		}

		void Reset()
		{
			vkResetFences(mDevice, 1, &mFence);
		}

		void Wait() const
		{
			vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);
		}

		VkFence GetHandle() const { return mFence; }

	private:
		VkDevice mDevice;
		VkFence mFence;
	};
}