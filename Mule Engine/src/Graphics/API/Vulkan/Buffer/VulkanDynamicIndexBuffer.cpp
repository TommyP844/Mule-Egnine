#include "Graphics/API/Vulkan/Buffer/VulkanDynamicIndexBuffer.h"

namespace Mule::Vulkan
{
	VulkanDynamicIndexBuffer::VulkanDynamicIndexBuffer(IndexType type, uint32_t indexCount)
		:
		DynamicIndexBuffer(type, indexCount),
		IVulkanBuffer(mBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_QUEUE_GRAPHICS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
	}

	VulkanDynamicIndexBuffer::~VulkanDynamicIndexBuffer()
	{
	}

	void VulkanDynamicIndexBuffer::SetData(const Buffer& buffer, uint32_t offset)
	{
		uint8_t* ptr = buffer.As<uint8_t>();
		memcpy(mMappedPtr, ptr + offset, buffer.GetSize());
	}

	void VulkanDynamicIndexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		uint8_t* ptr = (uint8_t*)data;
		memcpy(mMappedPtr, ptr + offset, size);
	}
}