#include "Graphics/Buffer/UniformBuffer.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

#include <set>
#include <vector>

namespace Mule
{
	UniformBuffer::UniformBuffer(WeakRef<GraphicsContext> context, uint32_t bytes)
		:
		IShaderBuffer(context->GetDevice()),
		mSize(bytes)
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
		info.size = mSize;
		info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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
		allocInfo.allocationSize = mSize;
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
	}

	UniformBuffer::~UniformBuffer()
	{
	}

	void UniformBuffer::SetData(void* data, uint32_t size, uint32_t offset)
	{
		void* gpuData;
		vkMapMemory(mDevice, mMemory, offset, size, 0, &gpuData);
		memcpy(gpuData, data, size);
		vkUnmapMemory(mDevice, mMemory);
	}

	Buffer UniformBuffer::ReadData(uint32_t offset, uint32_t size)
	{
		Buffer buffer(size);

		void* gpuData;
		vkMapMemory(mDevice, mMemory, offset, size, 0, &gpuData);
		memcpy(buffer.GetData(), gpuData, size);
		vkUnmapMemory(mDevice, mMemory);
		return buffer;
	}
}