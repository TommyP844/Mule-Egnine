#include "ECS/Scene.h"

#include "ECS/Entity.h"
#include "ECS/Components.h"

#include "Engine Context/EngineContext.h"
#include "Physics/Shape3D/BoxShape.h"
#include "Physics/Shape3D/SphereShape.h"
#include "Physics/Shape3D/CapsuleShape.h"
#include "Physics/Shape3D/PlaneShape.h"

#include "Scripting/ScriptContext.h"

#include <entt/entt.hpp>

#include <fstream>

namespace Mule
{
	Scene::Scene()
		: 
		Asset()
	{
		mRegistry.on_construct<CameraComponent>().connect<&Scene::OnCameraComponentConstruct>(this);

	}

	Entity Scene::CreateEntity(const std::string& name, const Guid& guid)
	{
		mModified = true;
		entt::entity eid = mRegistry.create();
		auto& meta = AddComponent<MetaComponent>(eid);
		meta.Name = name;
		meta.Guid = guid;
		AddComponent<RootComponent>(eid);
		AddComponent<TransformComponent>(eid);

		Entity e;
		e.mId = eid;
		e.mScene = this;

		mEntityLookup[guid] = eid;

		return e;
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		Entity e = CreateEntity(entity.Name());

		CopyComponent<RootComponent>(entity, entity);
		CopyComponent<MetaComponent>(entity, entity);
		CopyComponent<TransformComponent>(e, entity);
		CopyComponent<CameraComponent>(e, entity);
		CopyComponent<EnvironmentMapComponent>(e, entity);
		CopyComponent<PointLightComponent>(e, entity);
		CopyComponent<SpotLightComponent>(e, entity);
		CopyComponent<DirectionalLightComponent>(e, entity);
		CopyComponent<MeshComponent>(e, entity);
		CopyComponent<ScriptComponent>(e, entity);
		CopyComponent<RigidBodyComponent>(e, entity);
		CopyComponent<BoxColliderComponent>(e, entity);
		CopyComponent<SphereColliderComponent>(e, entity);
		CopyComponent<CapsuleColliderComponent>(e, entity);
		CopyComponent<PlaneColliderComponent>(e, entity);
		CopyComponent<RigidBodyConstraintComponent>(e, entity);

		for (auto child : entity.Children())
		{
			auto childEntity = CopyEntity(child);
			e.AddChild(childEntity);
		}

		return e;
	}

	template<typename T>
	inline void Scene::CopyComponent(Entity dst, Entity src)
	{
		if (src.HasComponent<T>())
		{
			if (!dst.HasComponent<T>())
				dst.AddComponent<T>();

			dst.GetComponent<T>() = src.GetComponent<T>();
		}
	}

	Ref<Scene> Scene::Copy()
	{
		auto scene = MakeRef<Scene>();
		scene->SetHandle(Handle());

		for (auto e : Iterate<RootComponent>())
		{
			CopyEntityToScene(scene, e);
		}

		return scene;
	}

	void Scene::DestroyEntity(Entity e)
	{
		auto iter = mEntityLookup.find(e.Guid());
		if (iter == mEntityLookup.end())
		{
			SPDLOG_ERROR("Failed to remove entity from scene: {}", Name());
			return;
		}

		mEntityLookup.erase(e.Guid());
		mModified = true;
		mRegistry.destroy(e.mId);
	}

	Entity Scene::GetEntityByGUID(Guid guid)
	{
		auto iter = mEntityLookup.find(guid);
		if (iter == mEntityLookup.end())
			return Entity();

		Entity e;
		e.mId = iter->second;
		e.mScene = this;

		return e;
	}

	bool Scene::IsEntityValid(entt::entity id)
	{
		return mRegistry.valid(id);
	}

	void Scene::OnPlayStart()
	{
		// Physics
		mPhysicsContext.Init();
		
		for (auto entity : mRegistry.view<RigidBodyComponent>())
		{
			Entity e(entity, this);

			auto& transformComponent = GetComponent<TransformComponent>(entity);
			auto& rigidbodyComponent = GetComponent<RigidBodyComponent>(entity);

			RigidBody3DInfo info;
			info.Guid = e.Guid();
			info.Position = transformComponent.Translation;
			info.Orientation = transformComponent.GetOrientation();
			info.Type = rigidbodyComponent.BodyType;
			info.CollisionLayers = UINT16_MAX;
			info.Mass = rigidbodyComponent.Mass;
			info.Shape = nullptr;

			if (HasComponent<SphereColliderComponent>(entity))
			{
				auto& collider = GetComponent<SphereColliderComponent>(entity);

				info.Shape = MakeRef<SphereShape>(collider.Radius, collider.Offset, collider.Trigger);
			}
			else if (HasComponent<BoxColliderComponent>(entity))
			{
				auto& collider = GetComponent<BoxColliderComponent>(entity);

				info.Shape = MakeRef<BoxShape>(collider.Extent, collider.Offset, collider.Trigger);
			}
			else if (HasComponent<CapsuleColliderComponent>(entity))
			{
				auto& collider = GetComponent<CapsuleColliderComponent>(entity);

				info.Shape = MakeRef<CapsuleShape>(collider.Radius, collider.HalfHeight, collider.Offset, collider.Trigger);
			}
			else if (HasComponent<PlaneColliderComponent>(entity))
			{
				auto& collider = GetComponent<PlaneColliderComponent>(entity);

				glm::mat3 transform = e.GetTransformTR();
				glm::vec3 normal = glm::vec3(0.f, 1.f, 0.f);
				glm::vec3 dir = glm::normalize(transform * normal);

				glm::vec4 plane = glm::vec4(dir, collider.Offset);

				info.Shape = MakeRef<PlaneShape>(plane, collider.Trigger);
			}

			if (!info.Shape)
				continue;

			mPhysicsContext.CreateRigidBody(info);	
		}

		auto scriptContext = mEngineContext->GetScriptContext();
		for (auto entity : mRegistry.view<ScriptComponent>())
		{
			Entity e(entity, this);

			const auto& scriptComponent = e.GetComponent<ScriptComponent>();
			
			scriptContext->CreateInstance(scriptComponent.ScriptName, e.Guid(), scriptComponent.Fields);
			scriptContext->OnStart(e.Guid());
		}
	}

	void Scene::OnPlayStop()
	{
		mPhysicsContext.Shutdown();
	}

	// TODO: get viewport width / height
	void Scene::OnUpdate(float dt)
	{
		mPhysicsContext.Step(dt);

		for (auto entity : mRegistry.view<RigidBodyComponent>())
		{
			auto& transform = GetComponent<TransformComponent>(entity);
			auto& rigidBodyComponent = GetComponent<RigidBodyComponent>(entity);
			auto& metaComponent = GetComponent<MetaComponent>(entity);

			transform.Translation = mPhysicsContext.GetPosition(metaComponent.Guid);
			transform.Rotation = mPhysicsContext.GetRotation(metaComponent.Guid);
		}

		for (auto entity : mRegistry.view<CameraComponent>())
		{
			Entity e(entity, this);

			auto& transformComponent = e.GetComponent<TransformComponent>();
			auto& cameraComponent = e.GetComponent<CameraComponent>();

			cameraComponent.Camera->SetPosition(transformComponent.Translation);
			cameraComponent.Camera->SetAspectRatio(mViewportWidth / mViewportHeight);
		}

		auto scriptContext = mEngineContext->GetScriptContext();
		for (auto entity : mRegistry.view<ScriptComponent>())
		{
			Entity e(entity, this);

			scriptContext->OnUpdate(e.Guid(), dt);

			if (HasComponent<RigidBodyComponent>(entity))
			{
				auto& transform = GetComponent<TransformComponent>(entity);
				auto& rigidBody3dComponent = GetComponent<RigidBodyComponent>(entity);
				auto& metaComponent = GetComponent<MetaComponent>(entity);

				mPhysicsContext.SetPosition(metaComponent.Guid, transform.Translation);
			}
		}
	}

	Entity Scene::CopyEntityToScene(WeakRef<Scene> scene, Entity entity)
	{
		auto& meta = entity.GetComponent<MetaComponent>();

		auto e = scene->CreateEntity(meta.Name, meta.Guid);

		CopyComponent<TransformComponent>(e, entity);
		CopyComponent<CameraComponent>(e, entity);
		CopyComponent<EnvironmentMapComponent>(e, entity);
		CopyComponent<PointLightComponent>(e, entity);
		CopyComponent<SpotLightComponent>(e, entity);
		CopyComponent<DirectionalLightComponent>(e, entity);
		CopyComponent<MeshComponent>(e, entity);
		CopyComponent<ScriptComponent>(e, entity);
		CopyComponent<RigidBodyComponent>(e, entity);
		CopyComponent<BoxColliderComponent>(e, entity);
		CopyComponent<SphereColliderComponent>(e, entity);
		CopyComponent<CapsuleColliderComponent>(e, entity);
		CopyComponent<PlaneColliderComponent>(e, entity);
		CopyComponent<RigidBodyConstraintComponent>(e, entity);

		for (auto child : entity.Children())
		{
			auto childEntity = CopyEntityToScene(scene, child);
			e.AddChild(childEntity);
		}

		return e;
	}

	void Scene::OnCameraComponentConstruct(entt::registry& registry, entt::entity id)
	{
		registry.get<CameraComponent>(id).Camera = MakeRef<Camera>();
	}
}