#include "Graphics/Renderer/RenderGraph/RenderGraph.h"
#include "Graphics/Renderer/RenderGraph/RenderPass.h"

#include "Graphics/Renderer/Renderer.h"

#include "ECS/Scene.h"

#include "Timer.h"

#include <spdlog/spdlog.h>

#include <unordered_map>
#include <unordered_set>

namespace Mule
{
	RenderGraph::RenderGraph()
		:
		mIsBaked(false)
	{
		mQueue = GraphicsQueue::Create();
	}

	RenderGraph::~RenderGraph()
	{

	}

	void RenderGraph::InitializeRegistry(ResourceRegistry& registry)
	{
		assert(mIsBaked && "Render Graph must be baked before calling InitializeRegistry()");

		for (auto pass : mPasses)
		{
			pass->InitRegistry(registry);
		}
	}

	void RenderGraph::Bake()
	{
		std::vector<Ref<RenderPass>> passesToCompile = mPasses;
		mPasses.clear();
		
		// List of Edges (A -> B)
		std::vector<std::pair<uint32_t, uint32_t>> edges;
		std::unordered_map<uint32_t, std::vector<uint32_t>> graph;
		std::unordered_map<uint32_t, uint32_t> inDegree;
		std::unordered_set<uint32_t> nodes;

		// Build Dependency Graph
		for (uint32_t i = 0; i < passesToCompile.size() - 1; i++)
		{
			for (uint32_t j = i + 1; j < passesToCompile.size(); j++)
			{
				auto passA = passesToCompile[i];
				auto passB = passesToCompile[j];

				auto depsA = passA->GetResourceUsage();
				auto depsB = passB->GetResourceUsage();


				for (auto resA : depsA)
				{
					for (auto resB : depsB)
					{
						ResourceHandle resAHandle = resA.first;
						ResourceHandle resBHandle = resB.first;

						ResourceAccess resAAccess = resA.second.Access;
						ResourceAccess resBAccess = resB.second.Access;

						if (resAHandle != resBHandle)
							continue;

						if (resAAccess == ResourceAccess::Read && resBAccess == ResourceAccess::Write)
						{
							edges.push_back({ j, i });
						}
						else if (resAAccess == ResourceAccess::Write && resBAccess == ResourceAccess::Read)
						{
							edges.push_back({ i, j });
						}
					}
				}
			}
		}

		for (const auto& edge : edges) {
			uint32_t from = edge.first;
			uint32_t to = edge.second;
			graph[from].push_back(to);
			inDegree[to]++;
			nodes.insert(from);
			nodes.insert(to);
		}

		std::queue<uint32_t> q;
		for (uint32_t node : nodes) {
			if (inDegree[node] == 0) {
				q.push(node);
			}
		}

		while (!q.empty()) {
			uint32_t current = q.front();
			q.pop();
			mPasses.push_back(passesToCompile[current]);

			for (uint32_t neighbor : graph[current]) {
				inDegree[neighbor]--;
				if (inDegree[neighbor] == 0) {
					q.push(neighbor);
				}
			}
		}

		if (mPasses.size() != nodes.size()) {
			SPDLOG_ERROR("failed to compiled graph, cycle detected");
			return;
		}

		SPDLOG_INFO("RenderGraph Compiled");

		std::unordered_set<ResourceHandle> frameBufferClears;

		for (auto pass : mPasses)
		{
			std::vector<BeginRenderingCommandAttachment> colorAttachments;
			BeginRenderingCommandAttachment depthAttachment;

			// Handle -> binding index
			std::vector<std::pair<ResourceHandle, uint32_t>> SRGHandles;

			PassType passType = pass->GetPassType();
			for (const auto& [handle, usage] : pass->GetResourceUsage())
			{
				ResourceAccess access = usage.Access;

				if (passType == PassType::Graphics)
				{
					if (access == ResourceAccess::Write)
					{
						if (handle.Type == ResourceType::RenderTarget)
						{
							pass->AddPreDrawCommand(TransitionLayoutCommand(handle, ImageLayout::ColorAttachment));
							colorAttachments.push_back({ handle, true, usage.Index });
						}
						else if (handle.Type == ResourceType::DepthAttachment)
						{
							pass->AddPreDrawCommand(TransitionLayoutCommand(handle, ImageLayout::DepthAttachment));
							depthAttachment = { handle, true };
						}
					}
					else if (access == ResourceAccess::Read)
					{
						if (handle.Type == ResourceType::RenderTarget || handle.Type == ResourceType::DepthAttachment)
							pass->AddPreDrawCommand(TransitionLayoutCommand(handle, ImageLayout::ShaderReadOnly));
					}

					if (handle.Type == ResourceType::ShaderResourceGroup)
					{
						SRGHandles.push_back({ handle, usage.Index });
					}
				}				
			}

			if (passType == PassType::Graphics)
			{
				std::sort(colorAttachments.begin(), colorAttachments.end(), [](const BeginRenderingCommandAttachment& lhs, const BeginRenderingCommandAttachment& rhs) {
					return lhs.index < rhs.index;
					});

				std::sort(SRGHandles.begin(), SRGHandles.end(), [](const std::pair<ResourceHandle, uint32_t>& lhs, const std::pair<ResourceHandle, uint32_t>& rhs) {
					return lhs.second < rhs.second;
					});

				pass->AddPreDrawCommand(BeginRenderingCommand(
					RegistryVariable::Width,
					RegistryVariable::Height,
					colorAttachments,
					depthAttachment
				));

				std::vector<ResourceHandle> SRGHandlesSorted;
				for (auto [handle, index] : SRGHandles)
					SRGHandlesSorted.push_back(handle);

				pass->AddPreDrawCommand(BindGraphicsPipelineCommand(
					pass->GetGraphicsPipeline(),
					SRGHandlesSorted
					));

				pass->AddPostDrawCommand(EndRenderingCommand());
			}
		}

		mIsBaked = true;
	}

	void RenderGraph::Execute(const CommandList& commands, const Camera& camera, uint32_t frameIndex)
	{
		assert(mIsBaked && "Render Graph must be baked before calling Execute");

		Ref<ResourceRegistry> registry = camera.GetRegistry();

		if (!registry)
		{
			SPDLOG_WARN("Camera Resource Registry not initialized");
			return;
		}

		Ref<TimelineSemaphore> semaphore = registry->GetSemaphore(frameIndex);
		uint64_t semaphoreValue = semaphore->GetValue();

		if (mPreExecutionCallback)
			mPreExecutionCallback(camera, frameIndex);

		if (registry->IsResizeRequested(frameIndex))
		{
			GraphicsContext::Get().AwaitIdle();

			auto [width, height] = registry->GetResizeDimensions(frameIndex);
			
			if(mResizeCallback)
				mResizeCallback(camera, frameIndex, width, height);

			registry->SetResizeHandled(frameIndex);
		}

		for (uint32_t i = 0; i < mPasses.size(); i++)
		{
			Ref<RenderPass> pass = mPasses[i];

			Ref<CommandBuffer> commandBuffer = pass->Execute(commands, *registry, frameIndex);
			Ref<Fence> fence = pass->GetFence(*registry, frameIndex);

			if (i == mPasses.size() - 1)
			{
				auto renderTarget = registry->GetColorOutput(frameIndex);
				commandBuffer->TranistionImageLayout(renderTarget, ImageLayout::ShaderReadOnly);
			}

			commandBuffer->End();

			mQueue->Submit(commandBuffer, semaphore, semaphoreValue, semaphoreValue + 1, fence);
			semaphoreValue++;
		}
	}

	WeakRef<RenderPass> RenderGraph::CreatePass(const std::string& name, PassType type)
	{
		auto renderPass = MakeRef<RenderPass>(name, type);
		mPasses.push_back(renderPass);
		return renderPass;
	}
}