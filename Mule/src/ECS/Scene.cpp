#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include <entt/entt.hpp>
#include "Engine Context/EngineContext.h"

#include <fstream>

namespace Mule
{

	Entity Scene::CreateEntity(const std::string& name, const Guid& guid, uint32_t id)
	{
		mModified = true;
		entt::entity eid = entt::null;
		if (id == 0)
		{
			eid = mRegistry.create();
		}
		else 
		{
			eid = mRegistry.create((entt::entity)id);
		}
		auto& meta = AddComponent<MetaComponent>(eid);
		meta.Name = name;
		meta.Guid = guid;
		AddComponent<RootComponent>(eid);
		AddComponent<TransformComponent>(eid);

		return Entity((uint32_t)eid, WeakRef<Scene>(this));
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

		for (auto e : mRegistry.view<MetaComponent>())
		{
			auto oldEntity = Entity((uint32_t)e, this);

			auto& meta = mRegistry.get<MetaComponent>(e);
			
			auto entity = scene->CreateEntity(meta.Name, meta.Guid, oldEntity.ID());

			//CopyComponent<RootComponent>(entity, oldEntity);
			//CopyComponent<MetaComponent>(entity, oldEntity);
			CopyComponent<TransformComponent>(entity, oldEntity);
			CopyComponent<CameraComponent>(entity, oldEntity);
			CopyComponent<EnvironmentMapComponent>(entity, oldEntity);
			CopyComponent<PointLightComponent>(entity, oldEntity);
			CopyComponent<SpotLightComponent>(entity, oldEntity);
			CopyComponent<DirectionalLightComponent>(entity, oldEntity);
			CopyComponent<MeshComponent>(entity, oldEntity);
			CopyComponent<ScriptComponent>(entity, oldEntity);			
		}

		return scene;
	}

	void Scene::DestroyEntity(entt::entity id)
	{
		mModified = true;
		mRegistry.destroy(id);
	}

	void Scene::IterateRootEntities(std::function<void(Entity)> func)
	{
		auto view = mRegistry.view<RootComponent>();
		for (auto id : view)
		{
			Entity e((uint32_t)id, WeakRef<Scene>(this));
			func(e);
		}
	}
	
	bool Scene::IsEntityValid(entt::entity id)
	{
		return mRegistry.valid(id);
	}

	void Scene::OnPlayStart()
	{
		auto scriptContext = mEngineContext->GetScriptContext();
		for (auto entity : mRegistry.view<ScriptComponent>())
		{
			Entity e((uint32_t)entity, this);

			const auto& scriptComponent = e.GetComponent<ScriptComponent>();
			auto scriptHandle = scriptComponent.Handle;

			auto scriptInstance = scriptContext->GetScriptInstance(scriptHandle);

			if (!scriptInstance)
				continue;

			scriptInstance->OnStart();
		}
	}

	// TODO: get viewport width / height
	void Scene::OnUpdate(float dt)
	{
		for (auto entity : mRegistry.view<CameraComponent>())
		{
			Entity e((uint32_t)entity, this);

			auto& transformComponent = e.GetComponent<TransformComponent>();
			auto& cameraComponent = e.GetComponent<CameraComponent>();

			cameraComponent.Camera.SetPosition(transformComponent.Translation);
			cameraComponent.Camera.SetAspectRatio(mViewportWidth / mViewportHeight);
		}

		auto scriptContext = mEngineContext->GetScriptContext();
		for (auto entity : mRegistry.view<ScriptComponent>())
		{
			Entity e((uint32_t)entity, this);

			const auto& scriptComponent = e.GetComponent<ScriptComponent>();
			auto scriptHandle = scriptComponent.Handle;

			auto scriptInstance = scriptContext->GetScriptInstance(scriptHandle);
			
			if (!scriptInstance)
				continue;

			scriptInstance->OnUpdate(dt);
		}
	}
}