#include "Rendering/Mesh.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	Mesh::Mesh(const std::string& name, const Buffer& vertices, const Buffer& indices, AssetHandle defaultMaterialHandle, bool indice32Bit)
		: 
		mName(name),
		mVertexCount(0),
		mIndexCount(0),
		mTriangleCount(0),
		mDefaultMaterialHandle(defaultMaterialHandle)
	{

		mIndexCount = indices.GetSize() / sizeof(uint16_t);
		mTriangleCount = mIndexCount / 3;

	}
	
	Mesh::~Mesh()
	{

	}

	void Mesh::Bind()
	{
	}
}