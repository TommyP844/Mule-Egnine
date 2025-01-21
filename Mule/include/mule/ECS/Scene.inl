#pragma once

#include "WeakRef.h"

template<typename ...Components>
inline void Mule::Scene::IterateEntitiesWithComponents(std::function<void(Entity)> func)
{
	auto view = mRegistry.view<Components...>();
	for (auto id : view)
	{
		Entity e(id, WeakRef<Scene>(this));
		func(e);
	}
}