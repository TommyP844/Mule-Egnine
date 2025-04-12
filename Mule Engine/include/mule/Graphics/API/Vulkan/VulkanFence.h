#pragma once

#include <Volk/volk.h>

namespace Mule
{
	class VulkanFence
	{
	public:
		VulkanFence();
		~VulkanFence();

		void Reset();

		void Wait() const;

		VkFence GetHandle() const { return mFence; }

	private:
		VkDevice mDevice;
		VkFence mFence;
	};
}