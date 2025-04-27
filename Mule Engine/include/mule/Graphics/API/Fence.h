#pragma once

#include "Ref.h"

namespace Mule
{
	class Fence
	{
	public:
		static Ref<Fence> Create();
		virtual ~Fence() = default;
		virtual void Wait() = 0;
		virtual void Reset() = 0;

	protected:
		Fence() = default;
	};
}