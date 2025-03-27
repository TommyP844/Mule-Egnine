#pragma once

#include <vulkan/vulkan.h>

namespace Mule
{
	class IShaderBuffer
	{
	public:
		IShaderBuffer(VkDevice device)
			:
			mDevice(device),
			mBuffer(VK_NULL_HANDLE),
			mMemory(VK_NULL_HANDLE)
		{}
		virtual ~IShaderBuffer()
		{
			vkFreeMemory(mDevice, mMemory, nullptr);
			vkDestroyBuffer(mDevice, mBuffer, nullptr);
		}

		VkBuffer GetBuffer() const { return mBuffer; }

	protected:
		VkDevice mDevice;
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
	};
}