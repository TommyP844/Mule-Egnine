#pragma once

#include "Ref.h"
#include "Buffer.h"
#include "Graphics/API/GraphicsCore.h"

namespace Mule
{
	class IndexBuffer
	{
	public:
		static Ref<IndexBuffer> Create(const Buffer& buffer, IndexType type);
		
		virtual ~IndexBuffer() = default;

		IndexType GetIndexType() const { return mIndexBufferType; }
		uint32_t GetIndexCount() const { return mIndexCount; }
		uint32_t GetTraingleCount() const { return mTriangleCount; }
		
	protected:
		IndexBuffer(uint32_t bufferSize, IndexType type);

		uint32_t mIndexCount;
		uint32_t mTriangleCount;
		IndexType mIndexBufferType;
	};
}