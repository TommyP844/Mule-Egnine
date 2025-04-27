#pragma once

#include "Ref.h"
#include "ECS/Scene.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/RenderGraph/RenderGraph.h"

#include <string>
#include <vector>

namespace Mule::RenderGraph
{
	class RenderGraph;

	class IRenderPass
	{
	public:
		IRenderPass(const std::string& name, WeakRef<RenderGraph> graph);
		virtual ~IRenderPass() = default;

		void AddPassDependency(const std::string& dependentPass);

		const std::string& GetName() const { return mName; }
		const std::vector<std::string>& GetDependecies() const { return mDependencies; }

		virtual void Setup() = 0;
		virtual bool Validate() = 0;
		virtual void Render(Ref<CommandBuffer> cmd, WeakRef<Scene> scene) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

	protected:
		friend class RenderGraph;
		WeakRef<RenderGraph> mGraph;

	private:
		const std::string mName;
		std::vector<std::string> mDependencies;
	};
}