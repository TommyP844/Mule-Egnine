#pragma once

#include "WeakRef.h"
#include "Ref.h"
#include "Guid.h"
#include "Asset/Asset.h"
#include "Physics/PhysicsContext.h"
#include "Services/ServiceManager.h"
#include "Graphics/API/Texture2D.h"
#include "Graphics/Renderer/CommandList.h"

#include "Graphics/Camera.h"

#include <entt/entt.hpp>

#include <string>
#include <set>

namespace Mule
{
	class Entity;

	class Scene : public Asset<AssetType::Scene>
	{
	public:
		Scene(Ref<ServiceManager> serviceManager);
		~Scene();

		Entity CreateEntity(const std::string& name = "Entity", const Guid& guid = Guid());
		Entity CopyEntity(Entity entity);
		Entity GetEntityByGUID(Guid guid);
		Ref<Scene> Copy();
		void DestroyEntity(Entity e);

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

		
		void OnPrepare();
		void OnUnload();

		// Call this function to create runtime resources
		void OnPlayStart();
		void OnPlayStop();

		void OnUpdate(float dt);

		void OnEditorRender(WeakRef<Camera> editorCamera);
		void OnRender();



		void SetModified() { mModified = true; }
		void ClearModified() { mModified = false; }
		bool IsModified() const { return mModified; }

		PhysicsContext& GetPhysicsContext() { return mPhysicsContext; }
		
		Ref<Camera> GetMainCamera() const;

	private:
		float mViewportWidth = 1.f;
		float mViewportHeight = 1.f;
		Ref<ServiceManager> mServiceManager;		
		PhysicsContext mPhysicsContext;
		entt::registry mRegistry;
		std::unordered_map<Guid, entt::entity> mEntityLookup;
		bool mModified;
		CommandList mCommandList;

		template<typename T>
		static void CopyComponent(Entity dst, Entity src);

		static Entity CopyEntityToScene(WeakRef<Scene> scene, Entity entity);

		// Component Sinks
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity id);

		// Command Lists
		void RecordRuntimeDrawCommands();
		void RecordEditorDrawCommands();
	};

}
#include "Scene.inl"
