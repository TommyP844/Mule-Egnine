#pragma once

#include "ECS/Guid.h"
#include "ECS/Entity.h"

#include "Graphics/Camera.h"

#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

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
		bool InheritTranslation = true;
		bool InheritRotation = true;
		bool InheritScale = true;
		glm::vec3 Translation;
		glm::vec3 Rotation;
		glm::vec3 Scale = glm::vec3(1.f);

		glm::mat4 TRS()
		{
			glm::mat4 translation = glm::translate(Translation);
			glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(Rotation)));
			glm::mat4 scale = glm::scale(Scale);
			return translation * rotation * scale;
		}
	};

	struct CameraComponent
	{
		bool Active = true;
		Camera Camera;
	};

	struct EnvironmentMapComponent
	{
		EnvironmentMapComponent() = default;
		EnvironmentMapComponent(const EnvironmentMapComponent& other) = default;
		bool Active = true;
		float Radiance = 1.f;
		AssetHandle EnvironmentMap = NullAssetHandle;
		bool DisplayIrradianceMap = false;
	};

	struct PointLightComponent
	{
		bool Active = true;
		float Radiance = 1.f;
		glm::vec3 Color = glm::vec3(1.f);
	};

	struct SpotLightComponent
	{
		bool Active = true;
		float Radiance = 1.f;
		float Angle = 45.f;
		glm::vec3 Color = glm::vec3(1.f);
	};

	struct DirectionalLightComponent
	{
		bool Active = true;
		float Intensity = 1.f;
		glm::vec3 Color = glm::vec3(1.f);
	};

	struct MeshComponent
	{
		bool Visible = true;
		AssetHandle MeshHandle;
		AssetHandle MaterialHandle;
		bool CastsShadows = true;
	};
}