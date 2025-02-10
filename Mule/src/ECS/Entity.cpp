#include "ECS/Entity.h"

#include "ECS/Components.h"

#include <spdlog/spdlog.h>

#include "Asset/Asset.h"
#include "ECS/Components.h"

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
			meta.Parent.RemoveChild(Entity((uint32_t)mId, mScene));
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
		child.GetComponent<MetaComponent>().Parent = Entity((uint32_t)mId, mScene);
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
		mScene->DestroyEntity(mId);
		mId = entt::null;
		mScene = nullptr;
	}

	void Entity::AddModelNodeRecursive(const ModelNode& node)
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

		for (const auto& childNode : node.GetChildren())
		{
			std::string nodeName = childNode.GetName().empty() ? "Node" : childNode.GetName();
			auto childEntity = mScene->CreateEntity(nodeName);
			AddChild(childEntity);
			childEntity.AddModelNodeRecursive(childNode);
		}
	}
}