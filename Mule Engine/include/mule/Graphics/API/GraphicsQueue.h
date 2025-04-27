#pragma once

#include "Ref.h"

#include "Fence.h"
#include "Semaphore.h"
#include "CommandBuffer.h"

namespace Mule
{

	class GraphicsQueue
	{
	public:
		static Ref<GraphicsQueue> Create();

		virtual ~GraphicsQueue() = default;

		virtual void Submit(Ref<CommandBuffer> commandBuffer, const std::vector<Ref<Semaphore>>& waitSemaphores, const std::vector<Ref<Semaphore>>& signalSemaphores, Ref<Fence> fence) = 0;

	protected:
		GraphicsQueue() = default;
	};
}
