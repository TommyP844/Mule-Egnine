#pragma once

#include "WeakRef.h"
#include "Scene.h"
#include "Entity.h"

namespace Mule
{
	template<typename ...Components>
	inline auto Scene::Iterate()
	{
		auto view = mRegistry.view<Components...>();
		return view.each() | std::views::transform([this](auto&& tuple) {
			auto entity = std::get<0>(tuple);
			return Entity(entity, this);
			});
	}
}

