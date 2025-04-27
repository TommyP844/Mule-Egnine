#pragma once

#include "Ref.h"
#include "Buffer.h"

namespace Mule
{
	class UniformBuffer
	{
	public:
		static Ref<UniformBuffer> Create(const Buffer& buffer);
		static Ref<UniformBuffer> Create(uint32_t size);

		virtual ~UniformBuffer() = default;

		virtual void SetData(const Buffer& buffer, uint32_t offset = 0) = 0;

		uint32_t GetSize() const { return mSize; }

	protected:
		UniformBuffer(uint32_t size);

		uint32_t mSize;
	};
}
