#include "Graphics/Mesh.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	Mesh::Mesh(WeakRef<GraphicsContext> context, const MeshDescription& description)
	{
		mVertexBuffer = MakeRef<VertexBuffer>(context, description.Vertices, description.VertexSize);
		mIndexBuffer = MakeRef<IndexBuffer>(context, description.Indices, description.IndexBufferType);
	}
	
	Mesh::~Mesh()
	{

	}

	void Mesh::Bind()
	{
	}
}