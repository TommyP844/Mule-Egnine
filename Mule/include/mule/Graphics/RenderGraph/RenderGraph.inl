#include "RenderGraph.h"
#pragma once

namespace Mule::RenderGraph
{
	template <typename T>
	void RenderGraph::AddResource(uint32_t frameIndex, const std::string& name, Ref<T> resource)
	{
		if (frameIndex >= mFrameCount)
		{
			SPDLOG_ERROR("FrameIndex out of range, valid values are 0 - {}", mFrameCount - 1);
			return;
		}

		auto& resources = mResources[frameIndex];
		auto iter = resources.find(name);
		
		if (iter != resources.end())
		{
			SPDLOG_WARN("RenderGraph Resource already exists: {}", name);
			return;
		}
		
		auto res = MakeRef<Resource<T>>(resource);
		resources[name] = res;
	}

	template<typename T>
	inline WeakRef<T> RenderGraph::GetResource(uint32_t frameIndex, const std::string& name) const
	{
		if (frameIndex >= mFrameCount)
		{
			SPDLOG_ERROR("FrameIndex out of range, valid values are 0 - {}", mFrameCount - 1);
			return nullptr;
		}

		auto& resources = mResources[frameIndex];
		auto iter = resources.find(name);

		if (iter == resources.end())
		{
			SPDLOG_WARN("RenderGraph failed to fine resource: {}", name);
			return nullptr;
		}
		Ref<Resource<T>> res = iter->second;
		return res->Get();
	}

	template<typename T>
	inline WeakRef<T> Mule::RenderGraph::RenderGraph::QueryResource(const std::string& name) const
	{
		const PerFrameData& perFrameData = mPerFrameData[mFrameIndex];
		const PassContext& ctx = perFrameData.Ctx;
		return ctx.Get<T>(name);
	}
}
