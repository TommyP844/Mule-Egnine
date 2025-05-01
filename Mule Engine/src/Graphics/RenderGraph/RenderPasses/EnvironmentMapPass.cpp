#include "Graphics/RenderGraph/RenderPasses/EnvironmentMapPass.h"

#include "ECS/Components.h"

#include "Graphics/EnvironmentMap.h"
#include "Asset/AssetManager.h"
#include "Graphics/ShaderFactory.h"
#include "Engine Context/EngineAssets.h"

namespace Mule
{
	EnvironmentMapPass::EnvironmentMapPass(Ref<ServiceManager> serviceManager, WeakRef<RenderGraph::RenderGraph> graph)
		:
		RenderGraph::IRenderPass("Environment Map Pass", graph),
		mServiceManager(serviceManager)
	{
		auto shaderFactory = mServiceManager->Get<ShaderFactory>();
		auto shader = shaderFactory->GetOrCreateGraphicsPipeline("EnvironmentMap");

		mResourceGroup = graph->AddResource<ShaderResourceGroup>(shader->GetBlueprintIndex(0));
	}

	void EnvironmentMapPass::Setup()
	{
		auto resourceGroup = mGraph->GetResource(mResourceGroup);
		WeakRef<UniformBuffer> cameraBuffer = mGraph->GetResource(mCameraBufferHandle);
		
		resourceGroup->Update(0, cameraBuffer);
	}

	bool EnvironmentMapPass::Validate()
	{
		bool valid = true;

		valid &= mCameraBufferHandle;
		valid &= mFramebufferHandle;

		return valid;
	}

	void EnvironmentMapPass::Render(Ref<CommandBuffer> cmd, WeakRef<Scene> scene)
	{
		auto assetManager = mServiceManager->Get<AssetManager>();
		auto shaderFactory = mServiceManager->Get<ShaderFactory>();

		WeakRef<EnvironmentMap> envMap = nullptr;
		for (auto entity : scene->Iterate<EnvironmentMapComponent>())
		{
			const auto& envMapComponent = entity.GetComponent<EnvironmentMapComponent>();
			if (!envMapComponent.Active)
				continue;

			envMap = assetManager->GetAsset<EnvironmentMap>(envMapComponent.EnvironmentMap);
		}

		if (!envMap)
			return;

		auto cubeMap = assetManager->GetAsset<TextureCube>(envMap->GetCubeMapHandle());
		auto shader = shaderFactory->GetOrCreateGraphicsPipeline("EnvironmentMap");
		auto mesh = assetManager->GetAsset<Mesh>(MULE_CUBE_MESH_HANDLE);
		auto resourceGroup = mGraph->GetResource(mResourceGroup);
		auto framebuffer = mGraph->GetResource(mFramebufferHandle);

		assert(shader && mesh && "Environment Map Pass Resources not found.");

		resourceGroup->Update(1, DescriptorType::Texture, cubeMap);

		cmd->BeginRendering(framebuffer, shader);

		cmd->BindPipeline(shader, { resourceGroup });
		cmd->BindAndDrawMesh(mesh, 1);

		cmd->EndRendering();

	}

	void EnvironmentMapPass::Resize(uint32_t width, uint32_t height)
	{
	}

	void EnvironmentMapPass::SetCameraBufferHandle(Mule::RenderGraph::ResourceHandle<UniformBuffer> cameraBufferHandle)
	{
		mCameraBufferHandle = cameraBufferHandle;
	}

	void EnvironmentMapPass::SetFramebufferHandle(Mule::RenderGraph::ResourceHandle<Framebuffer> framebufferHandle)
	{
		mFramebufferHandle = framebufferHandle;
	}
}