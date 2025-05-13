#pragma once

#include "Ref.h"

#include "Fence.h"
#include "Semaphore.h"
#include "CommandBuffer.h"
#include "TimelineSemaphore.h"

namespace Mule
{

	class GraphicsQueue
	{
	public:
		static Ref<GraphicsQueue> Create();

		virtual ~GraphicsQueue() = default;

		virtual void Submit(Ref<CommandBuffer> commandBuffer, const std::vector<Ref<Semaphore>>& waitSemaphores, const std::vector<Ref<Semaphore>>& signalSemaphores, Ref<Fence> fence) = 0;
		virtual void Submit(Ref<CommandBuffer> commandBuffer, Ref<TimelineSemaphore> semaphore, uint64_t waitValue, uint64_t signalValue, Ref<Fence> fence = nullptr) = 0;
		virtual void Submit(Ref<CommandBuffer> commandBuffer, Ref<TimelineSemaphore> waitSemaphore, uint64_t waitValue, Ref<TimelineSemaphore> signalSemaphore, uint64_t signalValue, Ref<Fence> fence = nullptr) = 0;

	protected:
		GraphicsQueue() = default;
	};
}
