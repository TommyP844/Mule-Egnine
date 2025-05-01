
#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

namespace Mule::Vulkan
{
	VulkanStagingBuffer::VulkanStagingBuffer(const Buffer& buffer)
		:
		StagingBuffer(),
		IVulkanBuffer(buffer.GetSize(),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_QUEUE_TRANSFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		WriteData(buffer);
	}

	VulkanStagingBuffer::VulkanStagingBuffer(uint32_t size)
		:
		StagingBuffer(),
		IVulkanBuffer(size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_QUEUE_TRANSFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
	}
	
	VulkanStagingBuffer::~VulkanStagingBuffer()
	{
	}
	
	void VulkanStagingBuffer::WriteData(const Buffer& buffer)
	{
		memcpy(mMappedPtr, buffer.GetData(), buffer.GetSize());
	}

	Buffer VulkanStagingBuffer::ReadData(uint32_t offset, uint32_t count)
	{
		Buffer buffer(count);

		memcpy(buffer.GetData(), (uint8_t*)mMappedPtr + offset, count);

		return buffer;
	}
}