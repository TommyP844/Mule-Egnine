#pragma once

#include "Ref.h"
#include "CommandBuffer.h"

namespace Mule
{
	class CommandAllocator
	{
	public:
		static Ref<CommandAllocator> Create();
		virtual ~CommandAllocator() = default;

		virtual void Reset() = 0;

		virtual Ref<CommandBuffer> CreateCommandBuffer() = 0;

	private:
	};
}