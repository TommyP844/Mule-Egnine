#pragma once

#include "Mesh.h"

#include <vector>

namespace Mule
{
	class ModelNode
	{
	public:
		ModelNode() {}
		~ModelNode() {}

		const std::vector<Ref<Mesh>>& GetMeshes() const { return mMeshes; }
		const std::vector<ModelNode>& GetChildren() const { return mChildren; }
		const glm::mat4& GetLocalTransform() const { return mLocalTransform; }
		const std::string& GetName() const { return mName; }

		void SetName(const std::string& name) { mName = name; }
		void AddMesh(const Ref<Mesh>& mesh) { mMeshes.push_back(mesh); }
		void AddChild(const ModelNode& child) { mChildren.push_back(child); }

		void SetLocalTransform(const glm::mat4& localTransform) { mLocalTransform = localTransform; }
	private:
		glm::mat4 mLocalTransform;

		std::string mName;
		std::vector<Ref<Mesh>> mMeshes;
		std::vector<ModelNode> mChildren;
	};
}