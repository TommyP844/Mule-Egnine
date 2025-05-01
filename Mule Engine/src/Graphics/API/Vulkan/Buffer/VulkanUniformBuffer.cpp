#include "Graphics/API/Vulkan/Buffer/VulkanUniformBuffer.h"

#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"
#include "Graphics/API/Vulkan/VulkanContext.h"

namespace Mule::Vulkan
{
	VulkanUniformBuffer::VulkanUniformBuffer(const Buffer& buffer)
		:
		UniformBuffer(buffer.GetSize()),
		IVulkanBuffer(
			buffer.GetSize(),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_QUEUE_GRAPHICS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		SetData(buffer);
	}
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size)
		:
		UniformBuffer(size),
		IVulkanBuffer(
			size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_QUEUE_GRAPHICS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
	}

	void VulkanUniformBuffer::SetData(const Buffer& buffer, uint32_t offset)
	{
		uint8_t* ptr = (uint8_t*)buffer.GetData();
		memcpy(mMappedPtr, ptr + offset, buffer.GetSize());
	}

}