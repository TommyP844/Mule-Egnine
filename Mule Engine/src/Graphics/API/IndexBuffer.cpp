
#include "Graphics/API/IndexBuffer.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Buffer/VulkanIndexBuffer.h"

namespace Mule
{
	Ref<IndexBuffer> IndexBuffer::Create(const Buffer& buffer, IndexType type)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::None:
			break;
		case Mule::GraphicsAPI::Vulkan:
			return Ref<Vulkan::VulkanIndexBuffer>(new Vulkan::VulkanIndexBuffer(buffer, type));
			break;
		default:
			break;
		}

		return nullptr;
	}

	IndexBuffer::IndexBuffer(uint32_t bufferSize, IndexType type)
		:
		mIndexBufferType(type)
	{
		switch (type)
		{
		case IndexType::Size_16Bit:
			mIndexCount = bufferSize / sizeof(uint16_t);
			break;
		case IndexType::Size_32Bit:
			mIndexCount = bufferSize / sizeof(uint32_t);
			break;
		default:
			assert("Invalid index type");
			break;
		}

		mTriangleCount = mIndexCount / 3;
	}
}