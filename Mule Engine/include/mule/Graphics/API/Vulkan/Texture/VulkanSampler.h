#pragma once

#include "Graphics/API/Sampler.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanSampler : Sampler
	{
	public:
		VulkanSampler(const SamplerDescription& description);
		~VulkanSampler();

		VkSampler GetSampler() const;

	private:
		VkSampler mSampler;
	};
}
