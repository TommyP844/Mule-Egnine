#pragma once

#include <Volk/volk.h>

#include <Graphics/API/Fence.h>

namespace Mule::Vulkan
{
	class VulkanFence : public Fence
	{
	public:
		VulkanFence();
		~VulkanFence();

		void Reset() override;
		void Wait() override;

		VkFence GetHandle() const { return mFence; }

	private:
		VkFence mFence;
	};
}