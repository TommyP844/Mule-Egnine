#pragma once

#include "Ref.h"
#include "Guid.h"
#include "Asset/Asset.h"

#include <entt/entt.hpp>

#include <string>
#include <set>

namespace Mule
{
	class Entity;

	class Scene : public Asset<AssetType::Scene>
	{
	public:
		Scene() : Asset() {}
		~Scene() {}

		Entity CreateEntity(const std::string& name = "Entity", const Guid& guid = Guid());

		void DestroyEntity(entt::entity id);

		void IterateRootEntities(std::function<void(Entity)> func);

		template<typename ...Components>
		void IterateEntitiesWithComponents(std::function<void(Entity)> func);

		template<typename T, typename ...Args>
		T& AddComponent(entt::entity id, Args&&... args)
		{
			mRegistry.emplace<T>(id, std::forward<Args>(args)...);
			return mRegistry.get<T>(id);
		}

		template<typename T>
		T& GetComponent(entt::entity id)
		{
			return mRegistry.get<T>(id);
		}

		template<typename T>
		void RemoveComponent(entt::entity id)
		{
			mRegistry.remove<T>(id);
		}

		template<typename T>
		bool HasComponent(entt::entity id) const
		{
			return mRegistry.all_of<T>(id);
		}

		bool IsEntityValid(entt::entity id);

		void OnUpdate(float dt);


	private:
		entt::registry mRegistry;
	};

}
#include "Scene.inl"
