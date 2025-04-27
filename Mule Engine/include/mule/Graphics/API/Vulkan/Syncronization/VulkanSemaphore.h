#pragma once

#include "Graphics/API/Semaphore.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanSemaphore : public Semaphore
	{
	public:
		VulkanSemaphore();
		~VulkanSemaphore();

		VkSemaphore GetHandle() const { return mSemaphore; }

	private:
		VkSemaphore mSemaphore;
	};
}