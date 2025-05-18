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

		if (mSetupCallback)
		{
			for (uint32_t i = 0; i < registry.GetFramesInFlight(); i++)
			{
				mSetupCallback(registry, i);
			}
		}
	}

	// TODO: currently were clearing render targets every time there written to, we only want to do this once when its first written to
	void RenderGraph::Bake()
	{
		std::vector<Ref<RenderPass>> passesToCompile = mPasses;
		mPasses.clear();
		
		// List of Edges (A -> B)
		std::vector<std::pair<uint32_t, uint32_t>> edges;
		std::unordered_map<uint32_t, std::vector<uint32_t>> graph;
		std::unordered_map<uint32_t, uint32_t> inDegree;
		std::unordered_set<uint32_t> nodes;
		std::unordered_map<std::string, uint32_t> namedIndices;

		for (uint32_t i = 0; i < passesToCompile.size(); i++) {
			nodes.insert(i);
			if (inDegree.find(i) == inDegree.end())
				inDegree[i] = 0;

			namedIndices[passesToCompile[i]->GetName()] = i;
		}

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

		for (auto pass : passesToCompile)
		{
			uint32_t to = namedIndices[pass->GetName()];
			for (auto dep : pass->GetDependencies())
			{
				assert(namedIndices.find(dep) != namedIndices.end() && "Dependency does not exist");
				uint32_t from = namedIndices[dep];

				edges.push_back({ from, to });
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

		std::unordered_set<ResourceHandle> clearedRenderTargets;

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

							bool clear = false;
							if (!clearedRenderTargets.contains(handle))
							{
								clear = true;
								clearedRenderTargets.insert(handle);
							}

							colorAttachments.push_back({ handle, clear, usage.Index });
						}
						else if (handle.Type == ResourceType::DepthAttachment)
						{
							pass->AddPreDrawCommand(TransitionLayoutCommand(handle, ImageLayout::DepthAttachment));

							bool clear = false;
							if (!clearedRenderTargets.contains(handle))
							{
								clear = true;
								clearedRenderTargets.insert(handle);
							}

							depthAttachment = { handle, clear };
						}
					}					
				}
				else if (passType == PassType::Compute)
				{
					if (access == ResourceAccess::Write)
					{
						if (handle.Type == ResourceType::RenderTarget || handle.Type == ResourceType::DepthAttachment)
						{
							pass->AddPreDrawCommand(TransitionLayoutCommand(handle, ImageLayout::General));

							if (!clearedRenderTargets.contains(handle))
							{
								pass->AddPreDrawCommand(ClearRenderTargetCommand(handle));
								clearedRenderTargets.insert(handle);
							}
						}
					}
				}

				if (handle.Type == ResourceType::ShaderResourceGroup)
				{
					SRGHandles.push_back({ handle, usage.Index });
				}

				if (access == ResourceAccess::Read)
				{
					if (handle.Type == ResourceType::RenderTarget || handle.Type == ResourceType::DepthAttachment)
						pass->AddPreDrawCommand(TransitionLayoutCommand(handle, ImageLayout::ShaderReadOnly));
				}
			}

			std::sort(SRGHandles.begin(), SRGHandles.end(), [](const std::pair<ResourceHandle, uint32_t>& lhs, const std::pair<ResourceHandle, uint32_t>& rhs) {
				return lhs.second < rhs.second;
				});

			std::vector<ResourceHandle> SRGHandlesSorted;
			for (auto [handle, index] : SRGHandles)
				SRGHandlesSorted.push_back(handle);

			if (passType == PassType::Graphics)
			{
				std::sort(colorAttachments.begin(), colorAttachments.end(), [](const BeginRenderingCommandAttachment& lhs, const BeginRenderingCommandAttachment& rhs) {
					return lhs.index < rhs.index;
					});

				pass->AddPreDrawCommand(BeginRenderingCommand(
					colorAttachments,
					depthAttachment
				));

				pass->AddPreDrawCommand(BindGraphicsPipelineCommand(
					pass->GetGraphicsPipeline(),
					SRGHandlesSorted
				));

				pass->AddPostDrawCommand(EndRenderingCommand());
			}
			else if (passType == PassType::Compute)
			{
				pass->AddPreDrawCommand(BindComputePipelineCommand(
					pass->GetComputePipeline(),
					SRGHandlesSorted
				));
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

		//registry->SetFrameIndex(frameIndex);
		

		Ref<TimelineSemaphore> semaphore = registry->GetSemaphore(frameIndex);
		uint64_t semaphoreValue = semaphore->GetValue();

		if (registry->IsResizeRequested(frameIndex))
		{
			registry->WaitForFences(frameIndex);

			auto [width, height] = registry->GetResizeDimensions(frameIndex);

			if (mResizeCallback)
				mResizeCallback(camera, frameIndex, width, height);

			registry->SetResizeHandled(frameIndex);
		}

		if (mPreExecutionCallback)
			mPreExecutionCallback(camera, commands, frameIndex);

		for (uint32_t i = 0; i < mPasses.size(); i++)
		{
			Ref<RenderPass> pass = mPasses[i];

			Ref<CommandBuffer> commandBuffer = pass->Execute(commands, *registry, frameIndex);
			Ref<Fence> fence = pass->GetFence(*registry, frameIndex);

			if (i == mPasses.size() - 1)
			{
				auto renderTargetHandle = registry->GetColorOutputHandle();
				auto renderTarget = registry->GetResource<Texture>(renderTargetHandle, frameIndex);
				commandBuffer->TranistionImageLayout(renderTarget, ImageLayout::ShaderReadOnly);
			}

			commandBuffer->End();

			mQueue->Submit(commandBuffer, semaphore, semaphoreValue, semaphoreValue + 1u, fence);
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