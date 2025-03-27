#include "Scripting/ScriptGlue.h"

#include "ECS/Entity.h"
#include "ECS/Scene.h"
#include "ECS/Components.h"


namespace Mule
{
	static WeakRef<EngineContext> gEngineContext = nullptr;

	void SetContext(WeakRef<EngineContext> context)
	{
		gEngineContext = context;
	}


#pragma region Entity
#define GET_COMPONENT_PTR(ref, ptr) { ptr = &ref; }

	void* GetComponentPtr(uint64_t guid, uint32_t componentId)
	{
		Entity e = gEngineContext->GetScene()->GetEntityByGUID(guid);
		if (!e)
		{
			SPDLOG_WARN("Invalid Entity Guid being passed to: {}", __FUNCTION__);
			return nullptr;
		}

		void* ptr = nullptr;

		switch (componentId)
		{
		case ROOT_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<RootComponent>(), ptr); break;
		case META_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<MetaComponent>(), ptr); break;
		case TRANSFORM_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<TransformComponent>(), ptr); break;
		case CAMERA_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<CameraComponent>(), ptr); break;
		case ENVIRONMENT_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<EnvironmentMapComponent>(), ptr); break;
		case POINT_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<PointLightComponent>(), ptr); break;
		case SPOT_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<SpotLightComponent>(), ptr); break;
		case DIRECTIONAL_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<DirectionalLightComponent>(), ptr); break;
		case MESH_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<MeshComponent>(), ptr); break;
		case SCRIPT_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.GetComponent<SpotLightComponent>(), ptr); break;
		case RIGID_BODY_3D_COMPONENT: GET_COMPONENT_PTR(e.GetComponent<RigidBody3DComponent>(), ptr); break;
		}

		return ptr;
	}

	void* AddComponentGetPtr(uint64_t guid, uint32_t componentId)
	{
		Entity e = gEngineContext->GetScene()->GetEntityByGUID(guid);
		if (!e)
		{
			SPDLOG_WARN("Invalid Entity Guid being passed to: {}", __FUNCTION__);
			return nullptr;
		}

		void* ptr = nullptr;

		switch (componentId)
		{
		case ROOT_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<RootComponent>(), ptr); break;
		case META_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<MetaComponent>(), ptr); break;
		case TRANSFORM_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<TransformComponent>(), ptr); break;
		case CAMERA_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<CameraComponent>(), ptr); break;
		case ENVIRONMENT_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<EnvironmentMapComponent>(), ptr); break;
		case POINT_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<PointLightComponent>(), ptr); break;
		case SPOT_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<SpotLightComponent>(), ptr); break;
		case DIRECTIONAL_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<DirectionalLightComponent>(), ptr); break;
		case MESH_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<MeshComponent>(), ptr); break;
		case SCRIPT_LIGHT_COMPONENT_ID: GET_COMPONENT_PTR(e.AddComponent<SpotLightComponent>(), ptr); break;
		}

		return ptr;
	}

	bool HasComponent(uint64_t guid, uint32_t componentId)
	{
		Entity e = gEngineContext->GetScene()->GetEntityByGUID(guid);
		if (!e)
		{
			SPDLOG_WARN("Invalid Entity Guid being passed to: {}", __FUNCTION__);
			return false;
		}

		switch (componentId)
		{
		case ROOT_COMPONENT_ID: return e.HasComponent<RootComponent>();
		case META_COMPONENT_ID: return e.HasComponent<MetaComponent>();
		case TRANSFORM_COMPONENT_ID: return e.HasComponent<TransformComponent>();
		case CAMERA_COMPONENT_ID: return e.HasComponent<CameraComponent>();
		case ENVIRONMENT_COMPONENT_ID: return e.HasComponent<EnvironmentMapComponent>();
		case POINT_LIGHT_COMPONENT_ID: return e.HasComponent<PointLightComponent>();
		case SPOT_LIGHT_COMPONENT_ID: return e.HasComponent<SpotLightComponent>();
		case DIRECTIONAL_LIGHT_COMPONENT_ID: return e.HasComponent<DirectionalLightComponent>();
		case MESH_LIGHT_COMPONENT_ID: return e.HasComponent<MeshComponent>();
		case SCRIPT_LIGHT_COMPONENT_ID: return e.HasComponent<SpotLightComponent>();
		}

		return false;
	}

	void RemoveComponent(uint64_t guid, uint32_t componentId)
	{
		Entity e = gEngineContext->GetScene()->GetEntityByGUID(guid);
		if (!e)
		{
			SPDLOG_WARN("Invalid Entity Guid being passed to: {}", __FUNCTION__);
			return;
		}

		switch (componentId)
		{
		case ROOT_COMPONENT_ID: e.RemoveComponent<RootComponent>(); break;
		case META_COMPONENT_ID: e.RemoveComponent<MetaComponent>(); break;
		case TRANSFORM_COMPONENT_ID: e.RemoveComponent<TransformComponent>(); break;
		case CAMERA_COMPONENT_ID: e.RemoveComponent<CameraComponent>(); break;
		case ENVIRONMENT_COMPONENT_ID: e.RemoveComponent<EnvironmentMapComponent>(); break;
		case POINT_LIGHT_COMPONENT_ID: e.RemoveComponent<PointLightComponent>(); break;
		case SPOT_LIGHT_COMPONENT_ID: e.RemoveComponent<SpotLightComponent>(); break;
		case DIRECTIONAL_LIGHT_COMPONENT_ID: e.RemoveComponent<DirectionalLightComponent>(); break;
		case MESH_LIGHT_COMPONENT_ID: e.RemoveComponent<MeshComponent>(); break;
		case SCRIPT_LIGHT_COMPONENT_ID: e.RemoveComponent<SpotLightComponent>(); break;
		}
	}

#pragma endregion

#pragma region Input

	void SetMousePos(glm::vec2 pos)
	{
		auto window = gEngineContext->GetWindow();
		window->SetMousePos(pos);
	}

	float GetMousePosX()
	{
		auto window = gEngineContext->GetWindow();
		glm::vec2 pos = window->GetMousePos();
		return pos.x;
	}

	float GetMousePosY()
	{
		auto window = gEngineContext->GetWindow();
		glm::vec2 pos = window->GetMousePos();
		return pos.y;
	}

	bool IsMouseButtonPressed(uint32_t mouseButton)
	{
		auto window = gEngineContext->GetWindow();

		return window->IsMouseButtonDown((MouseButton)mouseButton);
	}

	bool IsKeyDown(uint32_t key)
	{
		auto window = gEngineContext->GetWindow();

		return window->IsKeyDown((KeyCode)key);
	}

#pragma endregion

#pragma region Camera

	void UpdateCameraVectors(Camera* camera)
	{
		camera->UpdateLocalVectors();
	}

	void UpdateCameraViewMatrix(Camera* camera)
	{
		camera->UpdateView();
	}

	void UpdateCameraProjectionMatrix(Camera* camera)
	{
		camera->UpdateProjection();
	}

	void UpdateCameraVPMatrix(Camera* camera)
	{
		camera->UpdateViewProjection();
	}

#pragma endregion

#pragma region Physics

	float GetRigidBody3DMass(PhysicsObjectHandle handle)
	{
		auto& physicsContext = gEngineContext->GetScene()->GetPhysicsContext();

		auto rigidBody = physicsContext.GetRigidBody(handle);

		if (!rigidBody)
		{
			SPDLOG_WARN("Invalid rigid body: {}", __FUNCTION__);
			return 0.f;
		}

		return rigidBody->GetMass();
	}

	void SetRigidBody3DMass(PhysicsObjectHandle handle, float mass)
	{
		auto& physicsContext = gEngineContext->GetScene()->GetPhysicsContext();

		auto rigidBody = physicsContext.GetRigidBody(handle);

		if (!rigidBody)
		{
			SPDLOG_WARN("Invalid rigid body: {}", __FUNCTION__);
			return;
		}

		rigidBody->SetMass(mass);
	}

	void MoveRigidBody3DKinematic(PhysicsObjectHandle handle, glm::vec3 position, glm::quat rotation, float dt)
	{
		auto& physicsContext = gEngineContext->GetScene()->GetPhysicsContext();

		auto rigidBody = physicsContext.GetRigidBody(handle);

		if (!rigidBody)
		{
			SPDLOG_WARN("Invalid rigid body: {}", __FUNCTION__);
			return;
		}

		rigidBody->MoveKinematic(position, rotation, dt);
	}

#pragma endregion
}
