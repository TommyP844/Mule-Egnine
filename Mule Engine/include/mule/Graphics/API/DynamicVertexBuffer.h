#pragma once

#include "Ref.h"
#include "Buffer.h"

#include "Graphics/VertexLayout.h"

namespace Mule
{
	class DynamicVertexBuffer
	{
	public:
		static Ref<DynamicVertexBuffer> Create(const VertexLayout& layout, uint32_t vertexCount);

		virtual ~DynamicVertexBuffer() = default;

		const VertexLayout& GetVertexLayout() const { return mLayout; }
		uint32_t GetVertexCount() const { return mVertexCount; }
		uint32_t GetVertexSize() const { return mVertexSize; }

		virtual void SetData(const Buffer& buffer, uint32_t offset = 0) = 0;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

	protected:
		DynamicVertexBuffer(const VertexLayout& layout, uint32_t vertexCount);

	protected:
		VertexLayout mLayout;
		uint32_t mVertexCount;
		uint32_t mVertexSize;
		uint32_t mBufferSize;
	};
}