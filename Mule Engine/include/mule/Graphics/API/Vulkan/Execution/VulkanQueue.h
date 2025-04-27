#pragma once

#include "Graphics/API/GraphicsQueue.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanQueue : public GraphicsQueue
	{
	public:
		VulkanQueue();
		virtual ~VulkanQueue();

		void Submit(Ref<CommandBuffer> commandBuffer, const std::vector<Ref<Semaphore>>& waitSemaphores, const std::vector<Ref<Semaphore>>& signalSemaphores, Ref<Fence> fence) override;

		VkQueue GetHandle() const { return mQueue; }

	private:
		VkQueue mQueue;
	};
}
