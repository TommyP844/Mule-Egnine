#include "Graphics/API/UniformBuffer.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Buffer/VulkanUniformBuffer.h"

namespace Mule
{
	Ref<UniformBuffer> UniformBuffer::Create(const Buffer& buffer)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanUniformBuffer>(buffer);
		case Mule::GraphicsAPI::None:
		default:
			nullptr;
		}
	}

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanUniformBuffer>(size);
		case Mule::GraphicsAPI::None:
		default:
			nullptr;
		}
	}

	UniformBuffer::UniformBuffer(uint32_t size)
		:
		mSize(size)
	{
	}
}