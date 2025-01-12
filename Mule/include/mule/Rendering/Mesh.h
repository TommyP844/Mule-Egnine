#pragma once

// Engine
#include "Buffer.h"
#include "Asset/Asset.h"

// Submodules
#include <glm/glm.hpp>

// STD
#include <string>

namespace Mule
{
	class Mesh
	{
	public:
		Mesh(const std::string& name, const Buffer& vertices, const Buffer& indices, AssetHandle defaultMaterialHandle, bool indice32Bit);
		~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;

		const std::string& GetName() const { return mName; }

		AssetHandle GetDefaultMaterialHandle() const { return mDefaultMaterialHandle; }

		void Bind();

	private:
		std::string mName;
		uint32_t mIndexCount;
		uint32_t mVertexCount;
		uint32_t mTriangleCount;

		AssetHandle mDefaultMaterialHandle;


	};
}