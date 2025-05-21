#include "Graphics/API/Vulkan/Buffer/VulkanDynamicVertexBuffer.h"

namespace Mule::Vulkan
{
	VulkanDynamicVertexBuffer::VulkanDynamicVertexBuffer(const VertexLayout& layout, uint32_t vertexCount)
		:
		DynamicVertexBuffer(layout, vertexCount),
		IVulkanBuffer(mBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_QUEUE_GRAPHICS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
	}

	VulkanDynamicVertexBuffer::~VulkanDynamicVertexBuffer()
	{
	}
	
	void VulkanDynamicVertexBuffer::SetData(const Buffer& buffer, uint32_t offset)
	{
		uint8_t* ptr = buffer.As<uint8_t>();
		memcpy(mMappedPtr, ptr + offset, buffer.GetSize());
	}

	void VulkanDynamicVertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		uint8_t* ptr = (uint8_t*)data;
		memcpy(mMappedPtr, ptr + offset, size);
	}
}