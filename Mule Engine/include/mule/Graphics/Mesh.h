#pragma once

// Engine
#include "WeakRef.h"
#include "Buffer.h"
#include "Asset/Asset.h"
#include "API/VertexBuffer.h"
#include "API/IndexBuffer.h"

// Submodules
#include <glm/glm.hpp>

// STD
#include <string>

namespace Mule
{
	class Mesh : public Asset<AssetType::Mesh>
	{
	public:
		static Ref<Mesh> Create(const std::string& name, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, AssetHandle defaultMaterialHandle);
		~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;

		AssetHandle GetDefaultMaterialHandle() const { return mDefaultMaterialHandle; }

		uint32_t GetVertexCount() const { return mVertexBuffer->GetVertexCount(); }
		uint32_t GetTriangleCount() const { return mIndexBuffer->GetTraingleCount(); }

		const WeakRef<IndexBuffer>& GetIndexBuffer() const { return mIndexBuffer; }
		const WeakRef<VertexBuffer>& GetVertexBuffer() const { return mVertexBuffer; }

	private:
		Mesh(const std::string& name, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, AssetHandle defaultMaterialHandle);
		Ref<VertexBuffer> mVertexBuffer;
		Ref<IndexBuffer> mIndexBuffer;

		AssetHandle mDefaultMaterialHandle;
	};
}