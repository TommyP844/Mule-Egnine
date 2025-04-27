#pragma once

#include "Graphics/API/Vulkan/Buffer/IVulkanBuffer.h"

#include "WeakRef.h"
#include "Buffer.h"
#include "Graphics/API/IndexBuffer.h"

#include <Volk/volk.h>

namespace Mule::Vulkan {

	class VulkanIndexBuffer : public IndexBuffer, public IVulkanBuffer
	{
	public:
		VulkanIndexBuffer(const Buffer& buffer, IndexType bufferType);
		virtual ~VulkanIndexBuffer(){}
	};
}