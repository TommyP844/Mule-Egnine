#pragma once

#include "ECS/Guid.h"
#include "ECS/Entity.h"

#include <string>


namespace Mule
{
	// Only Used for checking if entity is a root component
	struct RootComponent { bool root = true; };

	struct MetaComponent
	{
		std::string Name;
		Guid Guid;

		Entity Parent;
		std::vector<Entity> Children;
	};
}