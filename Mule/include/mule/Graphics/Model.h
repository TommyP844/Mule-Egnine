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

	private:
		ModelNode mNode;
	};
}