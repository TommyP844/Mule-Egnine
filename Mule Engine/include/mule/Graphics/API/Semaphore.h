#pragma once

#include "Ref.h"

namespace Mule
{
	class Semaphore
	{
	public:
		static Ref<Semaphore> Create();
		
		virtual ~Semaphore() = default;

	protected:
		Semaphore() = default;
	};
}