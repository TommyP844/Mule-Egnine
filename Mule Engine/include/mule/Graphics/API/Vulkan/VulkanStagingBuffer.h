#pragma once

#include "WeakRef.h"
#include "Buffer.h"

namespace Mule
{
	class GraphicsContext;

	class VulkanStagingBuffer
	{
	public:
		VulkanStagingBuffer(uint32_t size);
		virtual ~VulkanStagingBuffer();
		void SetData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
		
		Buffer ReadData();

	private:
		uint32_t mSize;
	};
}