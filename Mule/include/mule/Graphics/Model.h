#pragma once

#include "Asset/Asset.h"

#include "ModelNode.h"

namespace Mule
{
	class Model : public Asset<AssetType::Model>
	{
	public:
		Model() : Asset() {}
		Model(const fs::path& filepath) : Asset(filepath) {}
		Model(AssetHandle handle, const fs::path& filepath) : Asset(handle, filepath) {}

		const ModelNode& GetRootNode() const { return mNode; }
		void SetRootNode(const ModelNode& node) { mNode = node; }

		void SetMin(const glm::vec3& min) { mMin = min; }
		void SetMax(const glm::vec3& max) { mMax = max; }

		const glm::vec3& GetMin() const { return mMin; }
		const glm::vec3& GetMax() const { return mMax; }

	private:
		ModelNode mNode;
		glm::vec3 mMin;
		glm::vec3 mMax;
	};
}