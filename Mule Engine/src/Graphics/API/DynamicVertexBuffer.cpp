#include "Graphics/API/DynamicVertexBuffer.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Buffer/VulkanDynamicVertexBuffer.h"

namespace Mule
{
	Ref<DynamicVertexBuffer> DynamicVertexBuffer::Create(const VertexLayout& layout, uint32_t vertexCount)
	{
		GraphicsAPI api = GraphicsContext::Get().GetAPI();

		switch (api)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanDynamicVertexBuffer>(layout, vertexCount);
		case Mule::GraphicsAPI::None:
		default:
			assert("Invalid graphics api");
			break;
		}
	}

	DynamicVertexBuffer::DynamicVertexBuffer(const VertexLayout& layout, uint32_t vertexCount)
		:
		mLayout(layout),
		mVertexCount(vertexCount)
	{
		mVertexSize = mLayout.GetVertexSize();
		mBufferSize = mVertexCount * mVertexSize;
	}
}