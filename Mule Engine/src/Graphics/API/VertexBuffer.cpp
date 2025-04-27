#include "Graphics/API/VertexBuffer.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Buffer/VulkanVertexBuffer.h"

namespace Mule
{
	Ref<VertexBuffer> VertexBuffer::Create(const Buffer& buffer, const VertexLayout& vertexLayout)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case GraphicsAPI::None: return nullptr;
		case GraphicsAPI::Vulkan: return Ref<Vulkan::VulkanVertexBuffer>(new Vulkan::VulkanVertexBuffer(buffer, vertexLayout));
		}

		return nullptr;
	}

	VertexBuffer::VertexBuffer(uint32_t bufferSize, const VertexLayout& layout)
		:
		mBufferSize(bufferSize),
		mLayout(layout),
		mVertexSize(layout.GetVertexSize()),
		mVertexCount(bufferSize / mVertexSize)
	{
	}
}
