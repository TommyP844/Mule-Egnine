#pragma once

#include "Buffer.h"

namespace Mule
{
	class ScopedBuffer : public Buffer
	{
	public:
		ScopedBuffer() {}
		ScopedBuffer(size_t size)
		{
			Allocate(size);
		}

		~ScopedBuffer()
		{
			Release();
		}
	};
}