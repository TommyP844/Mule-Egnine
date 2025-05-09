
#include "Graphics/Renderer/Renderer.h"

#include "Graphics/GPUObjects.h"

// Render Passes
#include "Graphics/Renderer/RenderGraph/RenderPasses/GBufferPass.h"

namespace Mule
{
	Renderer* Renderer::sRenderer = nullptr;

	void Renderer::Init()
	{
		assert(!sRenderer && "Renderer has already been initialized");
		sRenderer = new Renderer();	
		sRenderer->BuildGraph();
	}

	void Renderer::Shutdown()
	{
		assert(sRenderer && "Renderer has not been initialized");
		delete sRenderer;
	}

	Renderer& Renderer::Get()
	{
		assert(sRenderer && "Renderer has not been initialized");
		return *sRenderer;
	}

	Ref<ResourceRegistry> Renderer::CreateResourceRegistry()
	{
		Ref<ResourceRegistry> registry = MakeRef<ResourceRegistry>(mFramesInFlight);

		mResourceBuilder.InitializeRegistryResources(*registry);
		mRenderGraph->InitializeRegistry(*registry);

		registry->CopyRegistryResources(*mGlobalRegistry);

		return registry;
	}

	void Renderer::Submit(const Camera& camera, const CommandList& commandList)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mRenderRequests.push_back({
			.Camera = camera,
			.RenderCommands = commandList.GetCommands(),
		});		
	}

	void Renderer::Render()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		for (auto renderRequest : mRenderRequests)
		{
			mRenderGraph->Execute(renderRequest.RenderCommands, renderRequest.Camera, mFrameIndex);
		}

		mRenderRequests.clear();

		mFrameIndex ^= 1;
	}

	void Renderer::BuildGraph()
	{
		mRenderGraph = MakeRef<RenderGraph>();

		mBindlessMaterialBuffer = mResourceBuilder.CreateGlobalResource("Bindless.Material.Buffer", ResourceType::UniformBuffer, (uint32_t)sizeof(GPU::Material) * 800u);
		
		mBindlessMaterialSRG = mResourceBuilder.CreateGlobalResource("Bindless.Material.SRG", {
			ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Fragment)
			});

		mBindlessTextureSRG = mResourceBuilder.CreateGlobalResource("Bindless.Texture.SRG", {
			ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Fragment, 4096)
			});

		auto geometryPass = mRenderGraph->CreatePass<GBufferPass>(mResourceBuilder);
		//geometryPass->AddPassDependency("");

		ResourceHandle outputHandle = mResourceBuilder.GetHandle("GBuffer");
		mResourceBuilder.SetFramebufferOutputHandle(outputHandle);
		
		mRenderGraph->Bake();

		mGlobalRegistry = MakeRef<ResourceRegistry>(mFramesInFlight, ResourceRegistryFlags::None);
		mResourceBuilder.InitializeGlobalResourceRegistry(*mGlobalRegistry);

		mRenderGraph->SetPreExecutionCallback([&](const Camera& camera, uint32_t frameIndex) {
			auto registry = camera.GetRegistry();
			ResourceHandle cameraBufferHandle = ResourceHandle::Create("CameraBuffer");
			auto uniformBuffer = registry->GetResource<UniformBuffer>(cameraBufferHandle, frameIndex);
			Buffer cameraBuffer = Buffer(sizeof(GPU::Camera));
			GPU::Camera* cameraBufferPtr = cameraBuffer.As<GPU::Camera>();
			cameraBufferPtr->ViewProjection = camera.GetViewProj();
			cameraBufferPtr->View = camera.GetView();
			cameraBufferPtr->Position = camera.GetPosition();
			cameraBufferPtr->ViewDirection = camera.GetForwardDir();
			uniformBuffer->SetData(cameraBuffer);
			});
	}
}