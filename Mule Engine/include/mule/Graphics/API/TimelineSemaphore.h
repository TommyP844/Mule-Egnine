#pragma once

#include "Ref.h"

namespace Mule
{
	class TimelineSemaphore
	{
	public:
		static Ref<TimelineSemaphore> Create();

		virtual uint64_t GetValue() const = 0;
	};
}
