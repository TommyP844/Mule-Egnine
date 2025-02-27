#pragma once

#include "ECS/Scene.h"
#include "Graphics/Camera.h"
#include "Graphics/Execution/CommandBuffer.h"

#include "Resource.h"
#include "WeakRef.h"
#include "Ref.h"

#include <string>
#include <unordered_map>

#include <spdlog/spdlog.h>

namespace Mule::RenderGraph
{
	class PassContext
	{
	public:
		PassContext(){}
		PassContext(const std::unordered_map<std::string, Ref<IResource>>& resources)
			:
			mResources(resources)
		{ }

		template<typename T>
		WeakRef<T> Get(const std::string& name) const
		{
			auto iter = mResources.find(name);
			if (iter == mResources.end())
			{
				SPDLOG_WARN("Failed to find resource: {}", name);
				return nullptr;
			}
			Ref<Resource<T>> resource = iter->second;
			return resource->Get();
		}

		void SetScene(WeakRef<Scene> scene) { mScene = scene; }
		WeakRef<Scene> GetScene() const { return mScene; }

		void SetCamera(const Camera& camera) { mCamera = camera; }
		const Camera& GetCamera() const { return mCamera; }

		void SetCommandBuffer(WeakRef<CommandBuffer> cmd) { mCommandBuffer = cmd; }
		WeakRef<CommandBuffer> GetCommandBuffer() const { return mCommandBuffer; }

	private:
		std::unordered_map<std::string, Ref<IResource>> mResources;
		WeakRef<Scene> mScene;
		WeakRef<CommandBuffer> mCommandBuffer;
		Camera mCamera;
	};
}