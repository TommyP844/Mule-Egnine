#pragma once

#include "Buffer.h"
#include "WeakRef.h"

namespace Mule
{
	class GraphicsContext;

	class VulkanUniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t bytes);
		virtual ~VulkanUniformBuffer();

		void SetData(const void* data, uint32_t size, uint32_t offset = 0);
		Buffer ReadData(uint32_t offset = 0, uint32_t size = 0);

		uint32_t GetSize() const { return mSize; }

	private:
		uint32_t mSize;
	};
}