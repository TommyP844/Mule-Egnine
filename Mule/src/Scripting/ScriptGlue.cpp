#include "Scripting/ScriptGlue.h"

#include "ECS/Entity.h"
#include "ECS/Scene.h"
#include "ECS/Components.h"

namespace Mule::Scripting
{
	static WeakRef<EngineContext> gEngineContext = nullptr;

	void SetContext(WeakRef<EngineContext> context)
	{
		gEngineContext = context;
	}

#define GET_COMPONENT_PTR(ref, ptr) { ptr = &ref; }

	void* GetComponentPtr(uint32_t entityId, uint32_t componentId)
	{
		auto scene = gEngineContext->GetScene();
		Entity e(entityId, scene);

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
		}

		return ptr;
	}

	void* AddComponentGetPtr(uint32_t entityId, uint32_t componentId)
	{
		auto scene = gEngineContext->GetScene();
		Entity e(entityId, scene);

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

	bool HasComponent(uint32_t entityId, uint32_t componentId)
	{
		auto scene = gEngineContext->GetScene();
		Entity e(entityId, scene);

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

	void RemoveComponent(uint32_t entityId, uint32_t componentId)
	{
		auto scene = gEngineContext->GetScene();
		Entity e(entityId, scene);

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

	void SetMousePos(glm::vec2 pos)
	{
		auto window = gEngineContext->GetWindow();
		window->SetMousePos(pos);
	}

	glm::vec2 GetMousePos()
	{
		auto window = gEngineContext->GetWindow();

		return window->GetMousePos();
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
}
