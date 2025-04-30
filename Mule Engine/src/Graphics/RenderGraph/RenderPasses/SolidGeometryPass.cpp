#include "Graphics/RenderGraph/RenderPasses/SolidGeometryPass.h"

#include "Asset/AssetManager.h"

#include "ECS/Components.h"

#include "Graphics/ShaderFactory.h"

#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"

#include "Engine Context/EngineAssets.h"

namespace Mule
{
	SolidGeometryPass::SolidGeometryPass(Ref<ServiceManager> serviceManager, WeakRef<RenderGraph::RenderGraph> graph)
		:
		mServiceManager(serviceManager),
		IRenderPass("Solid Geometry Pass", graph)
	{
		auto shaderFactory = mServiceManager->Get<ShaderFactory>();
		auto shader = shaderFactory->GetOrCreateGraphicsPipeline("Geometry");
		auto blueprint = shader->GetBlueprintIndex(0);
		std::vector<Ref<ShaderResourceBlueprint>> blueprints = {
			blueprint
		};
		mShaderResourceGroupHandle = mGraph->AddResource<ShaderResourceGroup>(blueprints);
	}

	bool SolidGeometryPass::Validate()
	{
		return true;
	}

	void SolidGeometryPass::Setup()
	{
		//TODO: Update descriptor sets
		auto resourceGroup = mGraph->GetResource(mShaderResourceGroupHandle);
		WeakRef<UniformBuffer> cameraBuffer = mGraph->GetResource(mCameraBufferHandle);

		resourceGroup->Update(0, cameraBuffer);
	}

	void SolidGeometryPass::Resize(uint32_t width, uint32_t height)
	{
	}

	void SolidGeometryPass::SetCameraBufferHandle(RenderGraph::ResourceHandle<UniformBuffer> bufferHandle)
	{
		mCameraBufferHandle = bufferHandle;
	}

	void SolidGeometryPass::SetFramebufferHandle(RenderGraph::ResourceHandle<Framebuffer> framebufferHandle)
	{
		mFramebufferHandle = framebufferHandle;
	}

	void SolidGeometryPass::SetBindlessTextureResourceHandle(RenderGraph::ResourceHandle<ShaderResourceGroup> resourceHandle)
	{
		mBindlessTextureResourceHandle = resourceHandle;
	}

	void SolidGeometryPass::Render(Ref<CommandBuffer> cmd, WeakRef<Scene> scene)
	{
		auto assetManager = mServiceManager->Get<AssetManager>();
		auto shaderFactory = mServiceManager->Get<ShaderFactory>();

		auto shader = shaderFactory->GetOrCreateGraphicsPipeline("Geometry");

		auto resourceGroup = mGraph->GetResource(mShaderResourceGroupHandle);
		auto bindlessResourceGroup = mGraph->GetResource(mBindlessTextureResourceHandle);
		auto frameBuffer = mGraph->GetResource(mFramebufferHandle);
		
		
		// Resource Prep
		{
			bool foundEnvMap = false;
			for (auto entity : scene->Iterate<EnvironmentMapComponent>())
			{
				const auto& envMapComponent = entity.GetComponent<EnvironmentMapComponent>();

				if (!envMapComponent.Active)
					continue;

				auto envMap = assetManager->GetAsset<EnvironmentMap>(envMapComponent.EnvironmentMap);
				if (!envMap)
					continue;

				auto irradianceMap = assetManager->GetAsset<Texture>(envMap->GetDiffuseIBLMap());
				auto prefilterMap = assetManager->GetAsset<Texture>(envMap->GetPreFilterMap());
				auto brdfLut = assetManager->GetAsset<Texture>(envMap->GetBRDFLutMap());

				resourceGroup->Update(3, irradianceMap);
				resourceGroup->Update(4, prefilterMap);
				resourceGroup->Update(5, brdfLut);
				
				foundEnvMap = true;

				break;
			}

			if (!foundEnvMap)
			{
				auto blackCube = assetManager->GetAsset<Texture>(MULE_BLACK_TEXTURE_CUBE_HANDLE);
				auto blackImage = assetManager->GetAsset<Texture>(MULE_BLACK_TEXTURE_HANDLE);

				resourceGroup->Update(3, blackCube, 0);
				resourceGroup->Update(4, blackCube, 0);
				resourceGroup->Update(5, blackImage, 0);
			}
		}

		cmd->TranistionImageLayout(frameBuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
		cmd->BeginRendering(frameBuffer, shader);
		cmd->BindPipeline(shader, { resourceGroup, bindlessResourceGroup });
		cmd->BlendEnable(0, false);

		for (auto entity : scene->Iterate<MeshComponent>())
		{
			const auto& meshComponent = entity.GetComponent<MeshComponent>();

			if (!meshComponent.Visible)
				continue;

			glm::mat4 transform = entity.GetTransform();

			auto mesh = assetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
			auto material = assetManager->GetAsset<Material>(meshComponent.MaterialHandle);

			uint32_t constants[4] = {
				0,
				0,
				0,
				0
			};

			cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(transform));
			cmd->SetPushConstants(shader, ShaderStage::Fragment, &constants, sizeof(uint32_t) * 4);
			cmd->BindAndDrawMesh(mesh, 1);
		}

		cmd->EndRendering();
	}
}
