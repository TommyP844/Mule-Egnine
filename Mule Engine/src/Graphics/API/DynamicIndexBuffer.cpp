#include "Graphics/API/DynamicIndexBuffer.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Buffer/VulkanDynamicIndexBuffer.h"

namespace Mule
{
	Ref<DynamicIndexBuffer> DynamicIndexBuffer::Create(IndexType type, uint32_t indexCount)
	{
		GraphicsAPI api = GraphicsContext::Get().GetAPI();

		switch (api)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanDynamicIndexBuffer>(type, indexCount);
		case Mule::GraphicsAPI::None:
		default:
			assert("Invalid graphics api");
			break;
		}
	}

	DynamicIndexBuffer::DynamicIndexBuffer(IndexType type, uint32_t indexCount)
		:
		mIndexCount(indexCount),
		mIndexType(type)
	{
		mIndexSize = type == IndexType::Size_16Bit ? 2 : 4;
		mBufferSize = mIndexCount * mIndexSize;
	}
}