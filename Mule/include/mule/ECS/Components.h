#pragma once

#include "ECS/Guid.h"
#include "ECS/Entity.h"

#include "Graphics/Camera.h"

#include <string>

#include <glm/glm.hpp>

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

	struct TransformComponent
	{
		glm::vec3 Translation;
		glm::vec3 Rotation;
		glm::vec3 Scale;
	};

	struct CameraComponent
	{
		bool Active = true;
		Camera Camera;
	};

	struct SkyLightComponent
	{
		bool Active = true;
	};

	struct MeshComponent
	{
		bool Visible = true;
		AssetHandle MeshHandle;
		AssetHandle MaterialHandle;
	};
}