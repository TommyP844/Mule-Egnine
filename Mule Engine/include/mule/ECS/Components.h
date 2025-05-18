#pragma once

#include "ECS/Guid.h"
#include "ECS/Entity.h"

#include "Graphics/Camera.h"
#include "Scripting/ScriptFieldInfo.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <unordered_map>


namespace Mule
{
#pragma region Internal
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

	struct HighlightComponent
	{
		HighlightComponent() = default;
		HighlightComponent(const HighlightComponent&) = default;
		glm::vec3 Color = glm::vec3(253.f / 255.f, 166.f / 255.f, 58.f / 255.f);
	};

#pragma endregion

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		glm::vec3 Translation;
		glm::vec3 Rotation;
		glm::vec3 Scale = glm::vec3(1.f);

		glm::mat4 TRS() const
		{
			glm::mat4 translation = glm::translate(Translation);
			glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(Rotation)));
			glm::mat4 scale = glm::scale(Scale);
			return translation * rotation * scale;
		}

		glm::mat4 GetTR() const
		{
			glm::mat4 translation = glm::translate(Translation);
			glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(Rotation)));
			return translation * rotation;
		}

		glm::quat GetOrientation()
		{
			return glm::quat(glm::radians(Rotation));
		}
	};

	struct CameraComponent
	{
		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

		bool Active = true;
		Ref<Camera> Camera;
	};

	struct EnvironmentMapComponent
	{
		EnvironmentMapComponent() = default;
		EnvironmentMapComponent(const EnvironmentMapComponent&) = default;
		bool Active = true;
		float Radiance = 1.f;
		AssetHandle EnvironmentMap = AssetHandle::Null();
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
		float FallOff = 5.f;
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

		// Internal
		uint32_t MaterialIndex = 0;
	};

	struct ScriptComponent
	{
		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;

		std::string ScriptName;
		std::unordered_map<std::string, ScriptFieldInfo> Fields;
	};

#pragma region Physics

	struct RigidBodyComponent
	{
		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
		float Mass = 1.f;
		BodyType BodyType = BodyType::Dynamic;
	};

	struct RigidBodyConstraintComponent
	{
		RigidBodyConstraintComponent() = default;
		RigidBodyConstraintComponent(const RigidBodyConstraintComponent&) = default;

		bool LockTranslationX = false;
		bool LockTranslationY = false;
		bool LockTranslationZ = false;

		bool LockRotationX = false;
		bool LockRotationY = false;
		bool LockRotationZ = false;
	};

	struct IBaseColliderComponent
	{
		glm::vec3 Offset = glm::vec3(0.f);
		bool Trigger = false;
	};

	struct SphereColliderComponent : IBaseColliderComponent
	{
		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent&) = default;
		float Radius = 1.f;
	};

	struct BoxColliderComponent : IBaseColliderComponent
	{
		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
		glm::vec3 Extent = glm::vec3(1.f);
	};

	struct PlaneColliderComponent : IBaseColliderComponent
	{
		PlaneColliderComponent() = default;
		PlaneColliderComponent(const PlaneColliderComponent&) = default;

		float Offset = 0.f;
	};

	struct CapsuleColliderComponent : IBaseColliderComponent
	{
		CapsuleColliderComponent() = default;
		CapsuleColliderComponent(const CapsuleColliderComponent&) = default;

		float Radius = 1.f;
		float HalfHeight = 1.f;
	};

#pragma endregion

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
#define RIGID_BODY_3D_COMPONENT 11

}