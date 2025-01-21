#include "Graphics/Buffer/VertexBuffer.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

#include <set>
#include <vector>

namespace Mule
{
	VertexBuffer::VertexBuffer(WeakRef<GraphicsContext> context, const Buffer& buffer, uint32_t vertexSize)
		:
		mDevice(context->GetDevice()),
		mBuffer(VK_NULL_HANDLE),
		mMemory(VK_NULL_HANDLE),
		mVertexSize(vertexSize),
		mVertexCount(buffer.GetSize() / mVertexSize)
	{

		std::set<uint32_t> uniqueQueueFamilyIndices = {
			context->GetGraphicsQueue()->GetQueueFamilyIndex(),
			context->GetComputeQueue()->GetQueueFamilyIndex(),
			context->GetTransferQueue()->GetQueueFamilyIndex()
		};

		std::vector<uint32_t> queueFamilyIndices = std::vector<uint32_t>(uniqueQueueFamilyIndices.begin(), uniqueQueueFamilyIndices.end());

		VkBufferCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.size = buffer.GetSize();
		info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = queueFamilyIndices.size();
		info.pQueueFamilyIndices = queueFamilyIndices.data();

		VkResult result = vkCreateBuffer(mDevice, &info, nullptr, &mBuffer);
		if(result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mDevice, mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = buffer.GetSize();
		allocInfo.memoryTypeIndex = context->GetMemoryTypeIndex(memRequirements.memoryTypeBits, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &mMemory);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to allocate vertex buffer memory");
		}

		result = vkBindBufferMemory(mDevice, mBuffer, mMemory, 0);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed bind vertex buffer memory");
		}

		void* data;
		vkMapMemory(mDevice, mMemory, 0, buffer.GetSize(), 0, &data);
		memcpy(data, buffer.GetData(), buffer.GetSize());
		vkUnmapMemory(mDevice, mMemory);
	}

	VertexBuffer::~VertexBuffer()
	{
	}
}