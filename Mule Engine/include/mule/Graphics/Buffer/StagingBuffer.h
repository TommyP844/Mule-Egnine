#pragma once

#include "WeakRef.h"
#include "IShaderBuffer.h"
#include "Buffer.h"

namespace Mule
{
	class GraphicsContext;

	class StagingBuffer : public IShaderBuffer
	{
	public:
		StagingBuffer(WeakRef<GraphicsContext> context, uint32_t size);
		virtual ~StagingBuffer();
		void SetData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
		
		Buffer ReadData();

	private:
		uint32_t mSize;
	};
}