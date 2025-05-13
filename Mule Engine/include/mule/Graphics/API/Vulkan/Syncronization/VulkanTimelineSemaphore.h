#pragma once

#include "Graphics/API/TimelineSemaphore.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanTimelineSemaphore : public TimelineSemaphore
	{
	public:
		VulkanTimelineSemaphore();
		~VulkanTimelineSemaphore();

		uint64_t GetValue() const override;
		VkSemaphore GetSemaphore() const { return mSemaphore; }

	private:
		VkSemaphore mSemaphore;
	};
}