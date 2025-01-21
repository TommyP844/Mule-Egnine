#pragma once

// Engine
#include "Buffer.h"
#include "Asset/Asset.h"
#include "Buffer/VertexBuffer.h"
#include "Buffer/IndexBuffer.h"

// Submodules
#include <glm/glm.hpp>

// STD
#include <string>

namespace Mule
{
	struct MeshDescription
	{
		std::string Name;
		Buffer Vertices;
		uint32_t VertexSize;
		Buffer Indices;
		IndexBufferType IndexBufferType;
	};

	class Mesh
	{
	public:
		Mesh(WeakRef<GraphicsContext> context, const MeshDescription& description);
		~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;

		const std::string& GetName() const { return mName; }

		AssetHandle GetDefaultMaterialHandle() const { return mDefaultMaterialHandle; }

		uint32_t GetVertexCount() const { return mVertexBuffer->GetVertexCount(); }
		uint32_t GetTriangleCount() const { return mIndexBuffer->GetTriangleCount(); }

		const WeakRef<IndexBuffer>& GetIndexBuffer() const { return mIndexBuffer; }
		const WeakRef<VertexBuffer>& GetVertexBuffer() const { return mVertexBuffer; }

		void Bind();

	private:
		std::string mName;
		Ref<VertexBuffer> mVertexBuffer;
		Ref<IndexBuffer> mIndexBuffer;

		AssetHandle mDefaultMaterialHandle;


	};
}