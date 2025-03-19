#pragma once

#include "ECS/Guid.h"
#include "ECS/Entity.h"

#include "Graphics/Camera.h"
#include "Scripting/ScriptContext.h"

#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Mule
{
	// Only Used for checking if entity is a root component
	struct RootComponent 
	{
		RootComponent() = default;
		RootComponent(const RootComponent&) = default;

		bool root = true; 
	};

	struct MetaComponent
	{
		MetaComponent() = default;
		MetaComponent(const MetaComponent&) = default;

		std::string Name;
		Guid Guid;

		Entity Parent;
		std::vector<Entity> Children;
	};

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

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
		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

		bool Active = true;
		Camera Camera;
	};

	struct EnvironmentMapComponent
	{
		EnvironmentMapComponent() = default;
		EnvironmentMapComponent(const EnvironmentMapComponent&) = default;
		bool Active = true;
		float Radiance = 1.f;
		AssetHandle EnvironmentMap = NullAssetHandle;
		bool DisplayIrradianceMap = false;
	};

	struct PointLightComponent
	{
		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;

		bool Active = true;
		float Radiance = 1.f;
		glm::vec3 Color = glm::vec3(1.f);
	};

	struct SpotLightComponent
	{
		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;

		bool Active = true;
		float Radiance = 1.f;
		float Angle = 45.f;
		glm::vec3 Color = glm::vec3(1.f);
	};

	struct DirectionalLightComponent
	{
		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;

		bool Active = true;
		float Intensity = 1.f;
		glm::vec3 Color = glm::vec3(1.f);
	};

	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;

		bool Visible = true;
		AssetHandle MeshHandle;
		AssetHandle MaterialHandle;
		bool CastsShadows = true;
	};

	struct ScriptComponent
	{
		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;

		ScriptHandle Handle;
	};

	// See MuleScriptEngine/Mule/Components/Component.cs for components ID's

#define ROOT_COMPONENT_ID 1
#define META_COMPONENT_ID 2
#define TRANSFORM_COMPONENT_ID 3
#define CAMERA_COMPONENT_ID 4
#define ENVIRONMENT_COMPONENT_ID 5
#define POINT_LIGHT_COMPONENT_ID 6
#define SPOT_LIGHT_COMPONENT_ID 7
#define DIRECTIONAL_LIGHT_COMPONENT_ID 8
#define MESH_LIGHT_COMPONENT_ID 9
#define SCRIPT_LIGHT_COMPONENT_ID 10

}