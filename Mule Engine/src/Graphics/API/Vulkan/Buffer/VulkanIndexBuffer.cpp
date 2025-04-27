#include "Graphics/API/Vulkan/Buffer/VulkanIndexBuffer.h"

#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

namespace Mule::Vulkan
{
	VulkanIndexBuffer::VulkanIndexBuffer(const Buffer& buffer, IndexType bufferType)
		:
		IVulkanBuffer(buffer.GetSize(), 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_QUEUE_GRAPHICS_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		IndexBuffer(buffer.GetSize(), bufferType)
	{
		VulkanContext& context = VulkanContext::Get();

		VulkanStagingBuffer stagingBuffer(buffer);

		auto cmd = context.BeginSingleTimeCommandBuffer();
		VulkanContext::Get().CopyBuffer(cmd, &stagingBuffer, this);
		context.EndSingleTimeCommandBuffer(cmd);
	}
}