#include "Rendering/Mesh.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	Mesh::Mesh(const std::string& name, const Buffer& vertices, const Buffer& indices, const bgfx::VertexLayout& layout, AssetHandle defaultMaterialHandle, bool indice32Bit)
		: 
		mName(name),
		mVertexCount(0),
		mIndexCount(0),
		mTriangleCount(0),
		mVertexBufferHandle(BGFX_INVALID_HANDLE),
		mIndexBufferHandle(BGFX_INVALID_HANDLE),
		mVertexLayoutHandle(BGFX_INVALID_HANDLE),
		mDefaultMaterialHandle(defaultMaterialHandle)
	{

		mVertexCount = vertices.GetSize() / layout.getStride();
		mIndexCount = indices.GetSize() / sizeof(uint16_t);
		mTriangleCount = mIndexCount / 3;

		// Create vertex buffer
		const bgfx::Memory* vertexMem = bgfx::copy(vertices.GetData(), vertices.GetSize());
		mVertexBufferHandle = bgfx::createVertexBuffer(vertexMem, layout);

		if (!bgfx::isValid(mVertexBufferHandle))
		{
			SPDLOG_ERROR("Failed to create vertex buffer for mesh: " + mName);
		}

		int flags = indice32Bit ? BGFX_BUFFER_INDEX32 : 0;
		const bgfx::Memory* indexMem = bgfx::copy(indices.GetData(), indices.GetSize());
		mIndexBufferHandle = bgfx::createIndexBuffer(indexMem, flags);

		if (!bgfx::isValid(mIndexBufferHandle))
		{
			bgfx::destroy(mVertexBufferHandle);
			SPDLOG_ERROR("Failed to create index buffer for mesh: " + mName);
		}
	}
	
	Mesh::~Mesh()
	{
		SPDLOG_INFO("Destroying mesh: {}", mName);
		if (bgfx::isValid(mVertexBufferHandle))
		{
			bgfx::destroy(mVertexBufferHandle);
		}

		if (bgfx::isValid(mIndexBufferHandle))
		{
			bgfx::destroy(mIndexBufferHandle);
		}
	}

	void Mesh::Bind()
	{
		bgfx::setVertexBuffer(0, mVertexBufferHandle);
		bgfx::setIndexBuffer(mIndexBufferHandle);
	}
}