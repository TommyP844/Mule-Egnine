#pragma once

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class IVulkanBuffer
	{
	public:
		IVulkanBuffer(uint32_t size, VkBufferUsageFlags usageFlags, VkQueueFlagBits requestedQueueType, VkMemoryPropertyFlags memoryProperties);
		virtual ~IVulkanBuffer();

		void* GetMappedPtr() const;

		VkBuffer GetBuffer() const { return mBuffer; }
		uint32_t GetSize() const { return mSize; }

	protected:
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
		void* mMappedPtr;

		const uint32_t mSize;
	};
}