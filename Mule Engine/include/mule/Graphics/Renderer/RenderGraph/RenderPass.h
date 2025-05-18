#pragma once

#include "Ref.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"
#include "Graphics/Renderer/RenderGraph/ResourceBuilder.h"
#include "Graphics/Renderer/RenderCommand.h"
#include "Graphics/Renderer/CommandList.h"

#include "Graphics/API/ComputePipeline.h"
#include "Graphics/API/GraphicsPipeline.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace Mule
{
	enum class ResourceAccess
	{
		Read,
		Write
	};

	struct ResourceUsage
	{
		ResourceAccess Access;
		uint32_t Index;
	};

	enum class PassType
	{
		Compute,
		Graphics
	};

	class RenderPass
	{
	public:
		explicit RenderPass(const std::string& name, PassType type);
		virtual ~RenderPass() = default;

		void InitRegistry(ResourceRegistry& registry);

		void AddPreDrawCommand(const RenderCommand& command);
		void AddPostDrawCommand(const RenderCommand& command);
		void AddDependency(const std::string& passDependency);

		PassType GetPassType() const { return mPassType; }
		const std::string& GetName() const { return mName; }
		const std::unordered_map<ResourceHandle, ResourceUsage>& GetResourceUsage() const { return mResourceUsage; }
		Ref<Fence> GetFence(const ResourceRegistry& registry, uint32_t frameIndex);
		WeakRef<GraphicsPipeline> GetGraphicsPipeline() const { return mGraphicsPipeline; }
		WeakRef<ComputePipeline> GetComputePipeline() const { return mComputePipeline; }
		const std::vector<std::string>& GetDependencies() const { return mDependencies; }

		Ref<CommandBuffer> Execute(const CommandList& commandList, const ResourceRegistry& registry, uint32_t frameIndex);

		void AddResource(ResourceHandle handle, ResourceAccess access, uint32_t index = 0);
		void AddCommandType(RenderCommandType type);

		void SetExecutionCallback(std::function<void(Ref<CommandBuffer>, const CommandList&, const ResourceRegistry&, uint32_t)> callback);
		void SetPipeline(WeakRef<GraphicsPipeline> pipeline);
		void SetPipeline(WeakRef<ComputePipeline> pipeline);


	private:
		const std::string mName;
		std::string mFenceName;
		std::string mCmdName;

		std::unordered_map<ResourceHandle, ResourceUsage> mResourceUsage;
		std::unordered_set<RenderCommandType> mCommandTypes;
		PassType mPassType;

		std::function<void(Ref<CommandBuffer>, const CommandList&, const ResourceRegistry&, uint32_t)> mExecutionCallback;

		ResourceHandle mFenceHandle;
		ResourceHandle mCommandBufferHandle;

		CommandList mPreDrawCommandList;
		CommandList mPostDrawCommandList;

		WeakRef<ComputePipeline> mComputePipeline;
		WeakRef<GraphicsPipeline> mGraphicsPipeline;

		std::vector<std::string> mDependencies;
	};
}