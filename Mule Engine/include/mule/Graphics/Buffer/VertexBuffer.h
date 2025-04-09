#pragma once

#include "WeakRef.h"
#include "Buffer.h"

#include <Volk/volk.h>

namespace Mule
{
	class GraphicsContext;

	class VertexBuffer
	{
	public:
		VertexBuffer(WeakRef<GraphicsContext> context, const Buffer& buffer, uint32_t vertexSize);
		~VertexBuffer();

		VkBuffer GetBuffer() const { return mBuffer; }
		uint32_t GetVertexCount() const { return mVertexCount; }
		uint32_t GetVertexSize() const { return mVertexSize; }

	private:
		VkDevice mDevice;
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;

		uint32_t mVertexCount;
		uint32_t mVertexSize;
	};
}