#pragma once

#include "IVulkanBuffer.h"

#include "Graphics/API/StagingBuffer.h"

namespace Mule::Vulkan
{
	class VulkanStagingBuffer : public StagingBuffer, public IVulkanBuffer
	{
	public:
		VulkanStagingBuffer(const Buffer& buffer);
		virtual ~VulkanStagingBuffer();

		void WriteData(const Buffer& buffer) override;
		Buffer ReadData(uint32_t offset = 0, uint32_t count = UINT32_MAX) override;
		
	private:
		uint32_t mSize;
	};
}