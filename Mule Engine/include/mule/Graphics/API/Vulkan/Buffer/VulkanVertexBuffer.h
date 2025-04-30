#pragma once

#include "WeakRef.h"
#include "Buffer.h"

#include "Graphics/API/VertexBuffer.h"
#include "IVulkanBuffer.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{

	class VulkanVertexBuffer : public VertexBuffer, public IVulkanBuffer
	{
	public:
		VulkanVertexBuffer(const Buffer& buffer, const VertexLayout& layout);
		~VulkanVertexBuffer();
	private:
	};
}