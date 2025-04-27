#pragma once

#include "Ref.h"
#include "Buffer.h"

#include "Graphics/VertexLayout.h"

namespace Mule
{
	class VertexBuffer
	{
	public:
		static Ref<VertexBuffer> Create(const Buffer& buffer, const VertexLayout& vertexLayout);

		virtual ~VertexBuffer() = default;

		const VertexLayout& GetVertexLayout() const { return mLayout; }
		uint32_t GetVertexCount() const { return mVertexCount; }
		uint32_t GetVertexSize() const { return mVertexSize; }

	protected:
		VertexBuffer(uint32_t bufferSize, const VertexLayout& layout);

		VertexLayout mLayout;
		uint32_t mBufferSize;
		uint32_t mVertexCount;
		uint32_t mVertexSize;
	};
}