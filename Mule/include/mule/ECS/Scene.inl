#pragma once

#include "WeakRef.h"
#include "Scene.h"
#include "Entity.h"

template<typename ...Components>
inline auto Mule::Scene::Iterate()
{
	return mRegistry.view<Components...>();
}

