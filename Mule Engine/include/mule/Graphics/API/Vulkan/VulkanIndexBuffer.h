#pragma once

#include "WeakRef.h"
#include "Buffer.h"

#include <Volk/volk.h>

namespace Mule
{
	enum class IndexBufferType : uint32_t
	{
		BufferSize_16Bit = VK_INDEX_TYPE_UINT16,
		BufferSize_32Bit = VK_INDEX_TYPE_UINT32
	};

	class GraphicsContext;

	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer(const Buffer& buffer, IndexBufferType bufferType);
		~VulkanIndexBuffer();

		VkBuffer GetBuffer() const { return mBuffer; }
		IndexBufferType GetBufferType() const { return mBufferType; }
		uint32_t GetTriangleCount() const { return mTriangleCount; }
		uint32_t GetIndexCount() const { return mIndexCount; }

	private:
		VkDevice mDevice;
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
		IndexBufferType mBufferType;
		uint32_t mTriangleCount;
		uint32_t mIndexCount;
	};
}