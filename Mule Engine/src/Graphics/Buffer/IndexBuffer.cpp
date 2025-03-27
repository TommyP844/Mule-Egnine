#include "Graphics/Buffer/IndexBuffer.h"

#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Execution/CommandBuffer.h"

#include <spdlog/spdlog.h>

#include <set>
#include <vector>

namespace Mule

{
	IndexBuffer::IndexBuffer(WeakRef<GraphicsContext> context, const Buffer& buffer, IndexBufferType bufferType)
		:
		mDevice(context->GetDevice()),
		mBuffer(VK_NULL_HANDLE),
		mMemory(VK_NULL_HANDLE),
		mBufferType(bufferType),
		mTriangleCount(0)
	{
		std::set<uint32_t> uniqueQueueFamilyIndices = {
			context->GetGraphicsQueue()->GetQueueFamilyIndex()
		};

		std::vector<uint32_t> queueFamilyIndices = std::vector<uint32_t>(uniqueQueueFamilyIndices.begin(), uniqueQueueFamilyIndices.end());

		VkBufferCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.size = buffer.GetSize();
		info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT_KHR;
		info.sharingMode = queueFamilyIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = queueFamilyIndices.size();
		info.pQueueFamilyIndices = queueFamilyIndices.data();

		VkResult result = vkCreateBuffer(mDevice, &info, nullptr, &mBuffer);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mDevice, mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = context->GetMemoryTypeIndex(memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
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

		switch (bufferType)
		{
		case Mule::IndexBufferType::BufferSize_32Bit:
			mIndexCount = buffer.GetSize() / sizeof(uint32_t);
			break;
		case Mule::IndexBufferType::BufferSize_16Bit:
			mIndexCount = buffer.GetSize() / sizeof(uint16_t);
			break;
		default:
			break;
		}

		mTriangleCount = mIndexCount / 3;

		VkBufferCreateInfo stagingInfo{};
		stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingInfo.size = buffer.GetSize();
		stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer stagingBuffer;
		vkCreateBuffer(mDevice, &stagingInfo, nullptr, &stagingBuffer);

		VkMemoryRequirements stagingMemRequirements;
		vkGetBufferMemoryRequirements(mDevice, stagingBuffer, &stagingMemRequirements);

		VkMemoryAllocateInfo stagingAllocInfo{};
		stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		stagingAllocInfo.allocationSize = stagingMemRequirements.size;
		stagingAllocInfo.memoryTypeIndex = context->GetMemoryTypeIndex(
			stagingMemRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		VkDeviceMemory stagingMemory;
		vkAllocateMemory(mDevice, &stagingAllocInfo, nullptr, &stagingMemory);
		vkBindBufferMemory(mDevice, stagingBuffer, stagingMemory, 0);

		// (2) Copy data to the staging buffer
		void* data;
		vkMapMemory(mDevice, stagingMemory, 0, buffer.GetSize(), 0, &data);
		memcpy(data, buffer.GetData(), buffer.GetSize());
		vkUnmapMemory(mDevice, stagingMemory);

		auto queue = context->GetGraphicsQueue();
		auto commandPool = queue->CreateCommandPool();
		auto commandBuffer = commandPool->CreateCommandBuffer();

		commandBuffer->Begin();
		VkBufferCopy copyRegion{};
		copyRegion.size = buffer.GetSize();
		vkCmdCopyBuffer(commandBuffer->GetHandle(), stagingBuffer, mBuffer, 1, &copyRegion);
		commandBuffer->End();

		auto fence = context->CreateFence();
		fence->Reset();
		queue->Submit(commandBuffer, {}, {}, fence);
		fence->Wait();

		// (4) Cleanup staging buffer
		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingMemory, nullptr);
	}

	IndexBuffer::~IndexBuffer()
	{
		vkFreeMemory(mDevice, mMemory, nullptr);
		vkDestroyBuffer(mDevice, mBuffer, nullptr);
	}
}