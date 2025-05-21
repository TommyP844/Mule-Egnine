#pragma once

#include "Ref.h"
#include "Buffer.h"

#include "Graphics/API/GraphicsCore.h"

namespace Mule
{
	class DynamicIndexBuffer
	{
	public:
		static Ref<DynamicIndexBuffer> Create(IndexType type, uint32_t indexCount);

		virtual ~DynamicIndexBuffer() = default;

		uint32_t GetIndexCount() const { return mIndexCount; }
		uint32_t GetIndexSize() const { return mIndexSize; }
		IndexType GetIndexType() const { return mIndexType; }

		virtual void SetData(const Buffer& buffer, uint32_t offset = 0) = 0;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

	protected:
		DynamicIndexBuffer(IndexType type, uint32_t indexCount);

	protected:
		IndexType mIndexType;
		uint32_t mIndexCount;
		uint32_t mIndexSize;
		uint32_t mBufferSize;
	};
}