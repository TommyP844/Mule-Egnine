#pragma once

#include "Ref.h"
#include "WeakRef.h"
#include "Guid.h"
#include "Scene.h"

#include "entt/entt.hpp"

namespace Mule
{
	class Entity // : public ScriptableClass
	{
	public:
		Entity()
			:
			mId(entt::null),
			mScene(nullptr)
		{
		}

		Entity(entt::entity id, WeakRef<Scene> scene)
			:
			mId(id),
			mScene(scene)//,
			//ScriptableClass("Entity")
		{
			//SetFieldValue<int>("id", (int)mId);
			//scene ? SetFieldValue<Scene*>("scene", scene.Get()) : SetFieldValue("scene", 0);
		}

		WeakRef<Scene> GetScene() const { return mScene; }
		uint32_t ID() const { return (uint32_t)mId; }
		const std::string& Name();
		Guid Guid();

		Entity Parent();
		std::vector<Entity> Children();
		void Orphan();
		void RemoveChild(Entity child);
		void AddChild(Entity child);
		bool HasChildren();
		bool IsChild();
		
		void Destroy();

		template<typename T, typename ...Args>
		T& AddComponent(Args&&... args)
		{
			return mScene->AddComponent<T>(mId, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			return mScene->GetComponent<T>(mId);
		}

		template<typename T>
		bool HasComponent()
		{
			return mScene->HasComponent<T>(mId);
		}

		template<typename T>
		void RemoveComponent()
		{
			mScene->RemoveComponent<T>(mId);
		}

		bool operator==(const Entity& other)
		{
			return mId == other.mId && mScene == other.mScene;
		}

		bool operator==(const Entity& other) const
		{
			return mId == other.mId && mScene == other.mScene;
		}

		operator bool() {
			return mScene != nullptr && mId != entt::null;
		}

		bool operator <(const Entity& rhs) const
		{
			return mId < rhs.mId;
		}

	private:
		entt::entity mId;
		WeakRef<Scene> mScene;
	};
}