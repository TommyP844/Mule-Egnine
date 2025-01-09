#include "ECS/Entity.h"

#include "ECS/Components.h"

#include <spdlog/spdlog.h>

namespace Mule 
{
	const std::string& Entity::Name()
	{
		return GetComponent<MetaComponent>().Name;
	}

	Guid Entity::Guid()
	{
		return GetComponent<MetaComponent>().Guid;
	}

	Entity Entity::Parent()
	{
		auto& meta = GetComponent<MetaComponent>();
		return meta.Parent;
	}

	std::vector<Entity> Entity::Children()
	{
		auto& meta = GetComponent<MetaComponent>();
		return meta.Children;
	}

	void Entity::Orphan()
	{
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
		auto& meta = GetComponent<MetaComponent>();
		auto iter = std::find(meta.Children.begin(), meta.Children.end(), child);
		meta.Children.erase(iter);
		child.AddComponent<RootComponent>();
	}

	void Entity::AddChild(Entity child)
	{
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
}