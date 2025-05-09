#pragma once

#include "Ref.h"

#include "Graphics/Camera.h"
#include "Graphics/Renderer/RenderGraph/RenderGraph.h"
#include "Graphics/Renderer/RenderGraph/ResourceBuilder.h"
#include "CommandList.h"

#include <vector>
#include <mutex>

namespace Mule
{
	class Renderer
	{
	public:
		~Renderer() = default;

		static void Init();
		static void Shutdown();
		static Renderer& Get();

		// TODO: we need to keep a weak reference to all resource registrys so we can properly wait on them when updating global resources like bindless textures
		Ref<ResourceRegistry> CreateResourceRegistry();
		void Submit(const Camera& camera, const CommandList& commandList);

		void Render();

		uint32_t GetFramesInFlight() const { return mFramesInFlight; }
		uint32_t GetFrameIndex() const { return mFrameIndex; }

	private:
		Renderer() = default;
		void BuildGraph();

		static Renderer* sRenderer;

		struct RenderRequest
		{
			Camera Camera;
			std::vector<RenderCommand> RenderCommands;
		};

		std::mutex mMutex;
		std::vector<RenderRequest> mRenderRequests;
		Ref<RenderGraph> mRenderGraph;
		uint32_t mFramesInFlight = 2;
		uint32_t mFrameIndex = 0;

		ResourceBuilder mResourceBuilder;

		ResourceHandle mBindlessTextureSRG;
		ResourceHandle mBindlessMaterialBuffer;
		ResourceHandle mBindlessMaterialSRG;

		Ref<ResourceRegistry> mGlobalRegistry;
	};
}