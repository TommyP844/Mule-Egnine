#include "Graphics/API/Vulkan/Buffer/VulkanVertexBuffer.h"

#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

namespace Mule::Vulkan
{
	VulkanVertexBuffer::VulkanVertexBuffer(const Buffer& buffer, const VertexLayout& layout)
		:
		VertexBuffer(buffer.GetSize(), layout),
		IVulkanBuffer(
			buffer.GetSize(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_QUEUE_GRAPHICS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		VulkanContext& context = VulkanContext::Get();

		VulkanStagingBuffer stagingBuffer(buffer);

		auto cmd = context.BeginSingleTimeCommandBuffer();
 		VulkanContext::Get().CopyBuffer(cmd, &stagingBuffer, this);
		context.EndSingleTimeCommandBuffer(cmd);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		vkFreeMemory(device, mMemory, nullptr);
		vkDestroyBuffer(device, mBuffer, nullptr);
	}
}