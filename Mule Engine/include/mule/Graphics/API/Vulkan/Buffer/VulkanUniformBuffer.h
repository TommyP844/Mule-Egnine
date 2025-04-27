#pragma once

#include "Graphics/API/UniformBuffer.h"

#include "Graphics/API/Vulkan/Buffer/IVulkanBuffer.h"

namespace Mule::Vulkan
{
	class VulkanUniformBuffer : public UniformBuffer, public IVulkanBuffer
	{
	public:
		VulkanUniformBuffer(const Buffer& buffer);
		VulkanUniformBuffer(uint32_t size);
		virtual ~VulkanUniformBuffer() = default;

		void SetData(const Buffer& buffer, uint32_t offset = 0) override;
	};
}