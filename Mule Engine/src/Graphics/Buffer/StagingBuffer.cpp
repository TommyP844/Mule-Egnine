#include "Graphics/Buffer/StagingBuffer.h"
#include "Graphics/Context/GraphicsContext.h"
#include <spdlog/spdlog.h>

namespace Mule
{
	StagingBuffer::StagingBuffer(WeakRef<GraphicsContext> context, uint32_t size)
		:
		IShaderBuffer(context->GetDevice()),
		mSize(size)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create staging buffer");
		}
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mDevice, mBuffer, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = context->GetMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &mMemory) != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to allocate staging buffer memory");
		}
		vkBindBufferMemory(mDevice, mBuffer, mMemory, 0);
	}

	StagingBuffer::~StagingBuffer()
	{
	}

	void StagingBuffer::SetData(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		void* mappedData;
		vkMapMemory(mDevice, mMemory, offset, size, 0, &mappedData);
		memcpy(mappedData, data, size);
		vkUnmapMemory(mDevice, mMemory);
	}

	Buffer StagingBuffer::ReadData()
	{
		Buffer buffer(mSize);

		void* mappedData;
		vkMapMemory(mDevice, mMemory, 0, mSize, 0, &mappedData);
		memcpy(buffer.GetData(), mappedData, mSize);
		vkUnmapMemory(mDevice, mMemory);

		return buffer;
	}
}