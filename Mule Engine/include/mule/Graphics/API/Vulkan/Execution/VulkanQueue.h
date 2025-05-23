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
		void Submit(Ref<CommandBuffer> commandBuffer, Ref<TimelineSemaphore> semaphore, uint64_t waitValue, uint64_t signalValue, Ref<Fence> fence = nullptr) override;
		void Submit(Ref<CommandBuffer> commandBuffer, Ref<TimelineSemaphore> waitSemaphore, uint64_t waitValue, Ref<TimelineSemaphore> signalSemaphore, uint64_t signalValue, Ref<Fence> fence = nullptr) override;

		VkQueue GetHandle() const { return mQueue; }

	private:
		VkQueue mQueue;
	};
}
