#pragma once

#include "Graphics/API/CommandAllocator.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanCommandPool : public CommandAllocator
	{
	public:
		VulkanCommandPool();
		~VulkanCommandPool();

		void Reset() override;
		Ref<CommandBuffer> CreateCommandBuffer() override;

		VkCommandPool GetHandle() const { return mCommandPool; }

	private:
		VkCommandPool mCommandPool;
	};
}