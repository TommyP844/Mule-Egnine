#pragma once

#include <Volk/volk.h>

namespace Mule
{
	class Semaphore
	{
	public:
		Semaphore(VkDevice device, VkSemaphore semaphore) : mDevice(device), mSemaphore(semaphore) {}
		~Semaphore()
		{
			vkDestroySemaphore(mDevice, mSemaphore, nullptr);
		}

		VkSemaphore GetHandle() const { return mSemaphore; }

	private:
		VkDevice mDevice;
		VkSemaphore mSemaphore;
	};
}