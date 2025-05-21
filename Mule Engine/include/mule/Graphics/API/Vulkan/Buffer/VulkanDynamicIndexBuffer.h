#pragma once

#include "Graphics/API/DynamicIndexBuffer.h"

#include "IVulkanBuffer.h"

namespace Mule::Vulkan
{
	class VulkanDynamicIndexBuffer : public DynamicIndexBuffer, public IVulkanBuffer
	{
	public:
		VulkanDynamicIndexBuffer(IndexType type, uint32_t indexCount);
		virtual ~VulkanDynamicIndexBuffer();

		void SetData(const Buffer& buffer, uint32_t offset = 0) override;
		void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	};
}
