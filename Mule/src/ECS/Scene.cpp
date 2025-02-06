#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include <entt/entt.hpp>

#include <fstream>

namespace Mule
{

	Entity Scene::CreateEntity(const std::string& name, const Guid& guid)
	{
		mModified = true;
		entt::entity id = mRegistry.create();
		auto& meta = AddComponent<MetaComponent>(id);
		meta.Name = name;
		meta.Guid = guid;
		AddComponent<RootComponent>(id);
		AddComponent<TransformComponent>(id);

		return Entity((uint32_t)id, WeakRef<Scene>(this));
	}

	void Scene::DestroyEntity(entt::entity id)
	{
		mModified = true;
		mRegistry.destroy(id);
	}

	void Scene::IterateRootEntities(std::function<void(Entity)> func)
	{
		auto view = mRegistry.view<RootComponent>();
		for (auto id : view)
		{
			Entity e((uint32_t)id, WeakRef<Scene>(this));
			func(e);
		}
	}
	
	bool Scene::IsEntityValid(entt::entity id)
	{
		return mRegistry.valid(id);
	}

	void Scene::OnUpdate(float dt)
	{
	}
}