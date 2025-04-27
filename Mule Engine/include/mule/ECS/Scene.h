#pragma once

#include "WeakRef.h"
#include "Ref.h"
#include "Guid.h"
#include "Asset/Asset.h"
#include "Physics/PhysicsContext.h"
#include "Services/ServiceManager.h"

#include <entt/entt.hpp>

#include <string>
#include <set>

namespace Mule::RenderGraph
{
	class RenderGraph;
}

namespace Mule
{
	class Entity;
	class EngineContext;

	class Scene : public Asset<AssetType::Scene>
	{
	public:
		Scene(Ref<ServiceManager> serviceManager);
		~Scene() {}

		Entity CreateEntity(const std::string& name = "Entity", const Guid& guid = Guid());
		Entity CopyEntity(Entity entity);
		Ref<Scene> Copy();
		void DestroyEntity(Entity e);
		Entity GetEntityByGUID(Guid guid);

		template<typename ...Components>
		auto Iterate();


		template<typename T, typename ...Args>
		T& AddComponent(entt::entity id, Args&&... args)
		{
			mModified = true;
			mRegistry.emplace<T>(id, std::forward<Args>(args)...);
			return mRegistry.get<T>(id);
		}

		template<typename T>
		T& GetComponent(entt::entity id)
		{
			return mRegistry.get<T>(id);
		}

		template<typename T>
		const T& GetComponent(entt::entity id) const
		{
			return mRegistry.get<T>(id);
		}

		template<typename T>
		void RemoveComponent(entt::entity id)
		{
			mModified = true;
			mRegistry.remove<T>(id);
		}

		template<typename T>
		bool HasComponent(entt::entity id) const
		{
			return mRegistry.all_of<T>(id);
		}

		bool IsEntityValid(entt::entity id);

		void SetViewportDimension(float width, float height);

		// States

		// Call this function to load all scene resources
		void OnPrepare();

		// Call this function to unload all scene resources
		void OnUnload();

		// Call this function to create runtime resources
		void OnPlayStart();

		// call this function to unload all runtime resources
		void OnPlayStop();

		void OnUpdate(float dt);

		void OnRender();


		void SetModified() { mModified = true; }
		void ClearModified() { mModified = false; }
		bool IsModified() const { return mModified; }

		PhysicsContext& GetPhysicsContext() { return mPhysicsContext; }
		Ref<RenderGraph::RenderGraph> GetRenderGraph() const { return mRenderGraph; }

	private:
		float mViewportWidth = 1.f;
		float mViewportHeight = 1.f;
		WeakRef<EngineContext> mEngineContext;
		Ref<ServiceManager> mServiceManager;
		
		PhysicsContext mPhysicsContext;
		Ref<RenderGraph::RenderGraph> mRenderGraph;

		entt::registry mRegistry;
		std::unordered_map<Guid, entt::entity> mEntityLookup;
		bool mModified;

		template<typename T>
		static void CopyComponent(Entity dst, Entity src);

		static Entity CopyEntityToScene(WeakRef<Scene> scene, Entity entity);

		// Component Sinks
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity id);
	};

}
#include "Scene.inl"
