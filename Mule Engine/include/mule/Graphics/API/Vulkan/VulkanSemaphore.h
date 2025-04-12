#pragma once

#include <Volk/volk.h>

namespace Mule
{
	class VulkanSemaphore
	{
	public:
		VulkanSemaphore();
		~VulkanSemaphore();

		VkSemaphore GetHandle() const { return mSemaphore; }

	private:
		VkDevice mDevice;
		VkSemaphore mSemaphore;
	};
}