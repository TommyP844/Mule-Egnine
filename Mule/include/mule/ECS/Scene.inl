#pragma once

#include "WeakRef.h"
#include "Scene.h"

template<typename ...Components>
inline void Mule::Scene::IterateEntitiesWithComponents(std::function<void(Entity)> func)
{
	auto view = mRegistry.view<Components...>();
	for (auto id : view)
	{
		Entity e((uint32_t)id, WeakRef<Scene>(this));
		func(e);
	}
}

template<typename ...Components>
inline auto Mule::Scene::Iterate()
{
	return mRegistry.view<Components...>();
}

