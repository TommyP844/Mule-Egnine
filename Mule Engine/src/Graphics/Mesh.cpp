#include "Graphics/Mesh.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	Mesh::Mesh(const std::string& name, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, AssetHandle defaultMaterialHandle)
		:
		Asset(name),
		mVertexBuffer(vertexBuffer),
		mIndexBuffer(indexBuffer),
		mDefaultMaterialHandle(defaultMaterialHandle)
	{
	}
	
	Ref<Mesh> Mesh::Create(const std::string& name, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, AssetHandle defaultMaterialHandle)
	{
		return Ref<Mesh>(new Mesh(name, vertexBuffer, indexBuffer, defaultMaterialHandle));
	}

	Mesh::~Mesh()
	{

	}

}