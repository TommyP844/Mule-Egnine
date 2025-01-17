#pragma once

#include <vulkan/vulkan.h>

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

		void Wait()
		{
			/*
			VkSemaphoreWaitInfo info{};

			uint64_t waitValue = UINT64_MAX;
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
			info.semaphoreCount = 1;
			info.pSemaphores = &mSemaphore;
			info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
			info.pValues = &waitValue;
			info.pNext = nullptr;

			vkWaitSemaphores(mDevice, &info, UINT64_MAX);
			*/
		}

		VkSemaphore GetHandle() const { return mSemaphore; }

	private:
		VkDevice mDevice;
		VkSemaphore mSemaphore;
	};
}