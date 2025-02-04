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
		mVertexSize(vertexSize)
	{
		mVertexCount = buffer.GetSize() / mVertexSize;
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
		info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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

		// (3) Copy from staging buffer to GPU-local buffer
		VkCommandBuffer commandBuffer = context->CreateSingleTimeCmdBuffer();
		VkBufferCopy copyRegion{};
		copyRegion.size = buffer.GetSize();
		vkCmdCopyBuffer(commandBuffer, stagingBuffer, mBuffer, 1, &copyRegion);
		context->SubmitSingleTimeCmdBuffer(commandBuffer);
		context->WaitForSingleTimeCommands();

		// (4) Cleanup staging buffer
		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingMemory, nullptr);
	}

	VertexBuffer::~VertexBuffer()
	{
		vkFreeMemory(mDevice, mMemory, nullptr);
		vkDestroyBuffer(mDevice, mBuffer, nullptr);
	}
}