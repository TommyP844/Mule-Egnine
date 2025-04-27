#pragma once

#include "Ref.h"
#include "Buffer.h"

namespace Mule
{
	class StagingBuffer
	{
	public:
		static Ref<StagingBuffer> Create(const Buffer& buffer);

		virtual ~StagingBuffer() = default;

		virtual void WriteData(const Buffer& buffer) = 0;
		virtual Buffer ReadData(uint32_t offset = 0, uint32_t count = UINT32_MAX) = 0;

	protected:
		StagingBuffer();
	};
}
