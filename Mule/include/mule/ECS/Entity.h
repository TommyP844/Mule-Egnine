#pragma once

#include "Ref.h"
#include "WeakRef.h"
#include "Guid.h"
#include "Scene.h"
#include "Graphics/Model.h"

#include "entt/entt.hpp"

namespace Mule
{
	struct TransformComponent;

	class Entity // : public ScriptableClass
	{
	public:
		Entity()
			:
			mId(entt::null),
			mScene(nullptr)
		{
		}

		Entity(uint32_t id, WeakRef<Scene> scene)
			:
			mId((entt::entity)id),
			mScene(scene)
		{
		}

		WeakRef<Scene> GetScene() const { return mScene; }
		uint32_t ID() const { return (uint32_t)mId; }
		const std::string& Name() const;
		Guid Guid() const;
		TransformComponent& GetTransformComponent();

		Entity Parent();
		const std::vector<Entity>& Children() const;
		void Orphan();
		void RemoveChild(Entity child);
		void AddChild(Entity child);
		bool HasChildren();
		bool IsChild();

		void AddModel(WeakRef<Model> model);
		
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
		const T& GetComponent() const
		{
			return mScene->GetComponent<T>(mId);
		}

		template<typename T>
		bool HasComponent()
		{
			return mScene->HasComponent<T>(mId);
		}

		template<typename T>
		bool HasComponent() const
		{
			return mScene->HasComponent<T>(mId);
		}

		template<typename T>
		void RemoveComponent()
		{
			mScene->RemoveComponent<T>(mId);
		}

		bool operator==(Entity& other)
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

		void AddModelNodeRecursive(const ModelNode& node);
	};
}