#include "ECS/Entity.h"

#include "ECS/Components.h"
#include "Asset/Asset.h"

#include <spdlog/spdlog.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace Mule 
{
	const std::string& Entity::Name() const
	{
		return GetComponent<MetaComponent>().Name;
	}

	Guid Entity::Guid() const
	{
		return GetComponent<MetaComponent>().Guid;
	}

	TransformComponent& Entity::GetTransformComponent()
	{
		auto& transform = GetComponent<TransformComponent>();
		return transform;
	}

	const TransformComponent& Entity::GetTransformComponent() const
	{
		return GetComponent<TransformComponent>();
	}

	glm::mat4 Entity::GetTransform() const
	{
		glm::mat4 transform = GetTransformRecursiveTR();
		transform = glm::scale(transform, GetTransformComponent().Scale);

		return transform;
	}

	glm::mat4 Entity::GetTransformTR() const
	{
		glm::mat4 transform = GetTransformRecursiveTR();

		return transform;
	}

	Entity Entity::Parent()
	{
		auto& meta = GetComponent<MetaComponent>();
		return meta.Parent;
	}

	const std::vector<Entity>& Entity::Children() const
	{
		auto& meta = GetComponent<MetaComponent>();
		return meta.Children;
	}

	void Entity::Orphan()
	{
		mScene->SetModified();
		auto& meta = GetComponent<MetaComponent>();
		if (meta.Parent)
		{
			meta.Parent.RemoveChild(Entity(mId, mScene));
			meta.Parent = Entity();
		}
		else
		{
			SPDLOG_ERROR("Entity {} does not have a parent", meta.Name);
		}
	}

	void Entity::RemoveChild(Entity child)
	{
		mScene->SetModified();
		auto& meta = GetComponent<MetaComponent>();
		auto iter = std::find(meta.Children.begin(), meta.Children.end(), child);
		meta.Children.erase(iter);
		child.AddComponent<RootComponent>();
	}

	void Entity::AddChild(Entity child)
	{
		mScene->SetModified();
		auto& meta = GetComponent<MetaComponent>();
		meta.Children.push_back(child);

		child.RemoveComponent<RootComponent>();
		child.GetComponent<MetaComponent>().Parent = Entity(mId, mScene);
	}

	bool Entity::HasChildren()
	{
		auto& meta = GetComponent<MetaComponent>();
		return !meta.Children.empty();
	}

	bool Entity::IsChild()
	{
		auto& meta = GetComponent<MetaComponent>();
		return meta.Parent;
	}

	bool Entity::HasChild(Entity child)
	{
		auto& meta = GetComponent<MetaComponent>();
		if (std::find(meta.Children.begin(), meta.Children.end(), child) != meta.Children.end())
		{
			return true;
		}
		return false;
	}

	void Entity::AddModel(WeakRef<Model> model)
	{
		mScene->SetModified();
		if (!model)
		{
			SPDLOG_WARN("Model is null");
			return;
		}

		auto& node = model->GetRootNode();
		AddModelNodeRecursive(node);
	}

	void Entity::Destroy()
	{
		Orphan();
		for (auto child : Children())
		{
			auto& meta = child.GetComponent<MetaComponent>();

			meta.Parent = Entity();
			child.AddComponent<RootComponent>();
		}
		mScene->DestroyEntity(Entity(mId, mScene));
		mId = entt::null;
		mScene = nullptr;
	}

	void Entity::AddModelNodeRecursive(const ModelNode& node)
	{
		if (node.GetMeshes().size() == 1)
		{
			auto mesh = node.GetMeshes()[0];
			auto& meshComponent = AddComponent<MeshComponent>();

			meshComponent.Visible = true;
			meshComponent.MeshHandle = mesh->Handle();
			meshComponent.MaterialHandle = mesh->GetDefaultMaterialHandle();
		}
		else
		{
			for (auto& mesh : node.GetMeshes())
			{
				std::string name = mesh->Name().empty() ? "Mesh" : mesh->Name();
				auto child = mScene->CreateEntity(name);
				AddChild(child);
				MeshComponent& meshComponent = child.AddComponent<MeshComponent>();

				meshComponent.Visible = true;
				meshComponent.MeshHandle = mesh->Handle();
				meshComponent.MaterialHandle = mesh->GetDefaultMaterialHandle();
			}
		}

		for (const auto& childNode : node.GetChildren())
		{
			std::string nodeName = childNode.GetName().empty() ? "Node" : childNode.GetName();
			auto childEntity = mScene->CreateEntity(nodeName);
			auto& transformComponent = childEntity.GetTransformComponent();
			glm::mat4 mat = childNode.GetLocalTransform();
			
			glm::vec3 scale, skew, translation;
			glm::quat rotation;
			glm::vec4 perspective;
			if (glm::decompose(mat, scale, rotation, translation, skew, perspective))
			{
				transformComponent.Translation = translation;
				transformComponent.Rotation = glm::degrees(glm::eulerAngles(rotation));
				transformComponent.Scale = scale;
			}

			AddChild(childEntity);
			childEntity.AddModelNodeRecursive(childNode);
		}
	}

	glm::mat4 Entity::GetTransformRecursiveTR() const
	{
		auto& meta = GetComponent<MetaComponent>();

		if (meta.Parent)
		{
			return meta.Parent.GetTransform() * GetComponent<TransformComponent>().GetTR();
		}

		return GetComponent<TransformComponent>().GetTR();
	}
}