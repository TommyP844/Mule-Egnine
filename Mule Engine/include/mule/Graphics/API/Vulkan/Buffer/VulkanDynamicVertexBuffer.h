#pragma once

#include "Graphics/API/DynamicVertexBuffer.h"

#include "IVulkanBuffer.h"

namespace Mule::Vulkan
{
	class VulkanDynamicVertexBuffer : public DynamicVertexBuffer, public IVulkanBuffer
	{
	public:
		VulkanDynamicVertexBuffer(const VertexLayout& layout, uint32_t vertexCount);
		virtual ~VulkanDynamicVertexBuffer();

		void SetData(const Buffer& buffer, uint32_t offset = 0) override;
		void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	};
}
