#pragma once

#include "Buffer.h"
#include "WeakRef.h"
#include "IShaderBuffer.h"

namespace Mule
{
	class GraphicsContext;

	class UniformBuffer : public IShaderBuffer
	{
	public:
		UniformBuffer(WeakRef<GraphicsContext> context, uint32_t bytes);
		virtual ~UniformBuffer();

		void SetData(void* data, uint32_t size, uint32_t offset = 0);
		Buffer ReadData(uint32_t offset = 0, uint32_t size = 0);

		uint32_t GetSize() const { return mSize; }

	private:
		uint32_t mSize;
	};
}