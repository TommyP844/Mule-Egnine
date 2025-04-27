#include "Graphics/API/Vulkan/Buffer/IVulkanBuffer.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include <vector>

#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	IVulkanBuffer::IVulkanBuffer(uint32_t size, VkBufferUsageFlags usageFlags, VkQueueFlagBits requestedQueueType, VkMemoryPropertyFlags memoryProperties)
		:
		mSize(size),
		mBuffer(VK_NULL_HANDLE),
		mMemory(VK_NULL_HANDLE),
		mMappedPtr(nullptr)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		uint32_t queueFamilyIndex = context.GetQueueFamilyIndex();

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.pNext = nullptr;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.size = size;
		info.usage = usageFlags;
		info.queueFamilyIndexCount = 1;
		info.pQueueFamilyIndices = &queueFamilyIndex;

		VkResult result = vkCreateBuffer(device, &info, nullptr, &mBuffer);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = context.GetMemoryTypeIndex(memRequirements.memoryTypeBits, memoryProperties);

		result = vkAllocateMemory(device, &allocInfo, nullptr, &mMemory);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to allocate buffer memory");
		}

		result = vkBindBufferMemory(device, mBuffer, mMemory, 0);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed bind buffer memory");
		}

		if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			vkMapMemory(device, mMemory, 0, mSize, 0, &mMappedPtr);
		}
	}

	IVulkanBuffer::~IVulkanBuffer()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		if (mMappedPtr)
		{
			vkUnmapMemory(device, mMemory);
		}

		vkFreeMemory(device, mMemory, nullptr);
		vkDestroyBuffer(device, mBuffer, nullptr);
	}

	void* IVulkanBuffer::GetMappedPtr() const
	{
		assert(mMappedPtr != nullptr && "Device memory is not host visible, make sure to pass VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT flag on buffer creation");
		return mMappedPtr;
	}
}