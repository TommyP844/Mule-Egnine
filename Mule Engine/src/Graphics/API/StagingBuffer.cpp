#include "Graphics/API/StagingBuffer.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

namespace Mule
{
	Ref<StagingBuffer> StagingBuffer::Create(const Buffer& buffer)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case GraphicsAPI::Vulkan: return Ref<Vulkan::VulkanStagingBuffer>(new Vulkan::VulkanStagingBuffer(buffer));
		case GraphicsAPI::None: 
		default:
			return nullptr;
		}
	}
	StagingBuffer::StagingBuffer()
	{
	}
}