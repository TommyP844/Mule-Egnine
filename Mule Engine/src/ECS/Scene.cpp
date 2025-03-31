#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "Engine Context/EngineContext.h"
#include "Physics/Shape3D/BoxShape.h"
#include "Physics/Shape3D/SphereShape.h"

#include <entt/entt.hpp>

#include <fstream>

namespace Mule
{
	Scene::Scene(WeakRef<EngineContext> context)
		: 
		Asset(), mEngineContext(context)
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
		Entity e = CreateEntity(entity.Name() + "-Copy");

		//CopyComponent<RootComponent>(entity, entity);
		//CopyComponent<MetaComponent>(entity, entity);
		CopyComponent<TransformComponent>(e, entity);
		CopyComponent<CameraComponent>(e, entity);
		CopyComponent<EnvironmentMapComponent>(e, entity);
		CopyComponent<PointLightComponent>(e, entity);
		CopyComponent<SpotLightComponent>(e, entity);
		CopyComponent<DirectionalLightComponent>(e, entity);
		CopyComponent<MeshComponent>(e, entity);
		CopyComponent<ScriptComponent>(e, entity);
		CopyComponent<RigidBody3DComponent>(e, entity);
		CopyComponent<BoxColliderComponent>(e, entity);
		CopyComponent<SphereColliderComponent>(e, entity);

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
		auto scene = MakeRef<Scene>(mEngineContext);
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
		mPhysicsContext3D.Init();
		
		for (auto entity : mRegistry.view<RigidBody3DComponent>())
		{
			Entity e(entity, this);

			auto& transformComponent = GetComponent<TransformComponent>(entity);
			auto& rigidbodyComponent = GetComponent<RigidBody3DComponent>(entity);

			RigidBody3DInfo info;
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

			if (!info.Shape)
				continue;

			rigidbodyComponent.Handle = mPhysicsContext3D.CreateRigidBody3D(info);	
		}

		auto scriptContext = mEngineContext->GetScriptContext();
		for (auto entity : mRegistry.view<ScriptComponent>())
		{
			Entity e(entity, this);

			const auto& scriptComponent = e.GetComponent<ScriptComponent>();
			auto scriptHandle = scriptComponent.Handle;

			auto scriptInstance = scriptContext->GetScriptInstance(scriptHandle);

			if (!scriptInstance)
				continue;

			scriptInstance->OnStart();
		}
	}

	void Scene::OnPlayStop()
	{
		mPhysicsContext3D.Shutdown();
	}

	// TODO: get viewport width / height
	void Scene::OnUpdate(float dt)
	{
		mPhysicsContext3D.Step(dt);

		for (auto entity : mRegistry.view<RigidBody3DComponent>())
		{
			auto& transform = GetComponent<TransformComponent>(entity);
			auto& rigidBodyComponent = GetComponent<RigidBody3DComponent>(entity);

			auto rigidBody = mPhysicsContext3D.GetRigidBody(rigidBodyComponent.Handle);

			transform.Translation = rigidBody->GetPosition();
			transform.Rotation = rigidBody->GetRotation();
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

			const auto& scriptComponent = e.GetComponent<ScriptComponent>();
			auto scriptHandle = scriptComponent.Handle;

			auto scriptInstance = scriptContext->GetScriptInstance(scriptHandle);
			
			if (!scriptInstance)
				continue;

			scriptInstance->OnUpdate(dt);

			if (HasComponent<RigidBody3DComponent>(entity))
			{
				auto& transform = GetComponent<TransformComponent>(entity);
				auto& rigidBody3dComponent = GetComponent<RigidBody3DComponent>(entity);

				auto rigidBody3d = mPhysicsContext3D.GetRigidBody(rigidBody3dComponent.Handle);
				rigidBody3d->SetPosition(transform.Translation);
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
		CopyComponent<RigidBody3DComponent>(e, entity);
		CopyComponent<BoxColliderComponent>(e, entity);
		CopyComponent<SphereColliderComponent>(e, entity);

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