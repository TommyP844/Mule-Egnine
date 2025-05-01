#include "Graphics/SceneRenderer.h"

#include "Graphics/VertexLayout.h"
#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/API/CommandAllocator.h"
#include "Graphics/API/UniformBuffer.h"

#include "ECS/Components.h"
#include "Timer.h"
#include "ScopedBuffer.h"

#include "Asset/AssetManager.h"

#include "Engine Context/EngineAssets.h"

#include "Graphics/GPUObjects.h"


// Passes
#include "Graphics/RenderGraph/RenderPasses/SolidGeometryPass.h"
#include "Graphics/RenderGraph/RenderPasses/EnvironmentMapPass.h"

namespace Mule
{	
	SceneRenderer::SceneRenderer(Ref<ServiceManager> serviceManager)
		:
		mServiceManager(serviceManager),
		RenderGraph::RenderGraph(serviceManager)
	{
		SPDLOG_INFO("Scene renderer created");

		// Resources
		{
			// Uniform Buffers
			mCameraBufferHandle = AddResource<UniformBuffer>(sizeof(Camera));
			mLightBufferHandle = AddResource<UniformBuffer>(sizeof(GPU::GPULightData));
			mMaterialBufferHandle = AddResource<UniformBuffer>(sizeof(GPU::GPUMaterial) * 800);

			// Framebuffers
			FramebufferDescription mainFramebufferDesc{
				.Width = 800,
				.Height = 600,
				.Layers = 1,
				.ColorAttachments = {
					FramebufferAttachment(TextureFormat::RGBA_32F),
					FramebufferAttachment(TextureFormat::RG_32UI),
				},
				.DepthAttachment = FramebufferAttachment(TextureFormat::D_32F)
			};

			mMainFramebufferHandle = AddResource<Framebuffer>(mainFramebufferDesc);

			// Shader Resource Groups
			std::vector<ShaderResourceDescription> shaderResourceDescriptions = {
				ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Fragment, 4096)
			};
			
			mBindlessTextureShaderResourceHandle = AddResource<ShaderResourceGroup>(shaderResourceDescriptions);
		}

		// Passes
		{
			auto environmentMapPass = CreatePass<EnvironmentMapPass>();
			environmentMapPass->SetCameraBufferHandle(mCameraBufferHandle);
			environmentMapPass->SetFramebufferHandle(mMainFramebufferHandle);
		
			auto geometryPass = CreatePass<SolidGeometryPass>();
			geometryPass->SetCameraBufferHandle(mCameraBufferHandle);
			geometryPass->SetFramebufferHandle(mMainFramebufferHandle);
			geometryPass->SetBindlessTextureResourceHandle(mBindlessTextureShaderResourceHandle);
			geometryPass->SetLightBufferHandle(mLightBufferHandle);
			geometryPass->SetMaterialBufferHandle(mMaterialBufferHandle);

			geometryPass->AddPassDependency(environmentMapPass->GetName());
		}

		// Callbacks
		{
			SetPreRenderCallback(std::bind(&SceneRenderer::PreRenderCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			SetPostRenderCallback(std::bind(&SceneRenderer::PostRenderCallback, this, std::placeholders::_1, std::placeholders::_2));
			SetResizeCallback(std::bind(&SceneRenderer::ResizeCallback, this, std::placeholders::_1, std::placeholders::_2));
		}

		Bake();

		// Index 0 will always be black texture

		auto assetManager = mServiceManager->Get<AssetManager>();
		auto blackTexture = assetManager->GetAsset<Texture>(MULE_BLACK_TEXTURE_HANDLE);
		mBindlessTextureArray.Insert(blackTexture->Handle(), blackTexture);
	}

	SceneRenderer::~SceneRenderer()
	{
		SPDLOG_INFO("Scene renderer shutdown");
	}

	Ref<Framebuffer> SceneRenderer::GetCurrentFrameBuffer() const
	{
		return GetResource(mMainFramebufferHandle);
	}

	void SceneRenderer::PreRenderCallback(Ref<CommandBuffer> commandBuffer, WeakRef<Scene> scene, WeakRef<Camera> cameraOverride)
	{		
		auto cameraUniformBuffer = GetResource(mCameraBufferHandle);
		if (cameraOverride)
		{
			ScopedBuffer cameraBuffer(sizeof(GPU::GPUCamera));
			GPU::GPUCamera* gpuCamera = cameraBuffer.As<GPU::GPUCamera>();
			memset(gpuCamera, 0, sizeof(GPU::GPUCamera));
			gpuCamera->CameraPos = cameraOverride->GetPosition();
			gpuCamera->View = cameraOverride->GetView();
			gpuCamera->Proj = cameraOverride->GetProj();

			cameraUniformBuffer->SetData(cameraBuffer);
		}
		else
		{
			for (auto entity : scene->Iterate<CameraComponent>())
			{
				const auto& cameraComponent = entity.GetComponent<CameraComponent>();
				if (cameraComponent.Active)
				{
					Ref<Camera> camera = cameraComponent.Camera;
					ScopedBuffer cameraBuffer(sizeof(GPU::GPUCamera));
					GPU::GPUCamera* gpuCamera = cameraBuffer.As<GPU::GPUCamera>();
					memset(gpuCamera, 0, sizeof(GPU::GPUCamera));
					gpuCamera->CameraPos = camera->GetPosition();
					gpuCamera->View = camera->GetView();
					gpuCamera->Proj = camera->GetProj();

					cameraUniformBuffer->SetData(cameraBuffer);
					break;
				}
			}
		}

		// Light Data
		{
			ScopedBuffer lightBuffer(sizeof(GPU::GPULightData));
			GPU::GPULightData* lightBufferPtr = lightBuffer.As<GPU::GPULightData>();
			memset(lightBufferPtr, 0, sizeof(GPU::GPULightData));
			auto lightUniformBuffer = GetResource(mLightBufferHandle);

			for (auto entity : scene->Iterate<DirectionalLightComponent>())
			{
				const auto& lightComponent = entity.GetComponent<DirectionalLightComponent>();
				if (lightComponent.Active)
				{

					glm::quat orientation = entity.GetTransformComponent().GetOrientation();

					lightBufferPtr->DirectionalLight.Intensity = lightComponent.Intensity;
					lightBufferPtr->DirectionalLight.Color = lightComponent.Color;
					lightBufferPtr->DirectionalLight.Direction = glm::normalize(orientation * glm::vec3(0.f, -1.f, 0.f));

					break;
				}
			}

			for (auto entity : scene->Iterate<PointLightComponent>())
			{
				const auto& lightComponent = entity.GetComponent<PointLightComponent>();
				if (lightComponent.Active)
				{
					uint32_t index = lightBufferPtr->NumPointLights++;

					if (index >= 1024)
					{
						SPDLOG_ERROR("Too many point lights in scene. Max is 1024");
						break;
					}

					lightBufferPtr->PointLights[index].Intensity = lightComponent.Radiance;
					lightBufferPtr->PointLights[index].Color = lightComponent.Color;
					lightBufferPtr->PointLights[index].Position = entity.GetTransformComponent().Translation;
				}
			}

			lightUniformBuffer->SetData(lightBuffer);
		}

		// Textures
		{
			auto shaderResourceGroup = GetResource(mBindlessTextureShaderResourceHandle);
			const auto& array = mBindlessTextureArray.GetArray();
			for (uint32_t i = 0; i < array.size(); i++)
			{
				shaderResourceGroup->Update(0, DescriptorType::Texture, array[i], i);
			}
		}

		// Materials
		{
			auto materialUniformBuffer = GetResource(mMaterialBufferHandle);
			const auto& array = mBindlessMaterialArray.GetArray();
			ScopedBuffer materialBuffer(sizeof(GPU::GPUMaterial) * array.size());
			memcpy(materialBuffer.GetData(), array.data(), sizeof(GPU::GPUMaterial) * array.size());
			materialUniformBuffer->SetData(materialBuffer);
		}

		commandBuffer->ClearFrameBuffer(GetResource(mMainFramebufferHandle));
	}

	void SceneRenderer::PostRenderCallback(Ref<CommandBuffer> commandBuffer, WeakRef<Scene> scene)
	{
		auto framebuffer = GetResource(mMainFramebufferHandle);
		commandBuffer->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly);
	}

	void SceneRenderer::ResizeCallback(uint32_t width, uint32_t height)
	{
		SPDLOG_INFO("SceneRenderer Resize Callback: {}x{}", width, height);

		auto framebuffer = GetResource(mMainFramebufferHandle);
		framebuffer->Resize(width, height);
	}

	uint32_t SceneRenderer::QueryOrInsertTexture(AssetHandle handle, uint32_t defaultIndex)
	{
		uint32_t index = mBindlessTextureArray.QueryIndex(handle);
		if (index == UINT32_MAX)
		{
			auto assetManager = mServiceManager->Get<AssetManager>();
			auto texture = assetManager->GetAsset<Texture>(handle);
			if (!texture)
				return defaultIndex;

			index = mBindlessTextureArray.Insert(handle, texture);
		}

		return index;
	}

	uint32_t SceneRenderer::AddTexture(WeakRef<Texture> texture)
	{
		uint32_t index = mBindlessMaterialArray.QueryIndex(texture->Handle());
		if (index == UINT32_MAX)
		{
			mBindlessTextureArray.Insert(texture->Handle(), texture);
		}
		else
		{
			SPDLOG_INFO("Texture already exists in bindless array");
		}

		return index;
	}

	uint32_t SceneRenderer::AddMaterial(WeakRef<Material> material)
	{
		uint32_t index = mBindlessMaterialArray.QueryIndex(material->Handle());
		if (index == UINT32_MAX)
		{
			GPU::GPUMaterial gpuMaterial;

			gpuMaterial.AlbedoColor = material->AlbedoColor;
			gpuMaterial.TextureScale = material->TextureScale;
			gpuMaterial.MetalnessFactor = material->MetalnessFactor;
			gpuMaterial.RoughnessFactor = material->RoughnessFactor;
			gpuMaterial.AOFactor = material->AOFactor;
			gpuMaterial.AlbedoIndex = QueryOrInsertTexture(material->AlbedoMap);
			gpuMaterial.NormalIndex = QueryOrInsertTexture(material->NormalMap);
			gpuMaterial.MetalnessIndex = QueryOrInsertTexture(material->MetalnessMap);
			gpuMaterial.RoughnessIndex = QueryOrInsertTexture(material->RoughnessMap);
			gpuMaterial.AOIndex = QueryOrInsertTexture(material->AOMap);
			gpuMaterial.EmissiveIndex = QueryOrInsertTexture(material->EmissiveMap);

			mBindlessMaterialArray.Insert(material->Handle(), gpuMaterial);
		}
		else
		{
			SPDLOG_INFO("Material already exists in bindless array");
		}

		return index;
	}

	void SceneRenderer::UpdateMaterial(WeakRef<Material> material)
	{
		uint32_t index = mBindlessMaterialArray.QueryIndex(material->Handle());
		if (index != UINT32_MAX)
		{
			GPU::GPUMaterial gpuMaterial;

			gpuMaterial.AlbedoColor = material->AlbedoColor;
			gpuMaterial.TextureScale = material->TextureScale;
			gpuMaterial.MetalnessFactor = material->MetalnessFactor;
			gpuMaterial.RoughnessFactor = material->RoughnessFactor;
			gpuMaterial.AOFactor = material->AOFactor;
			gpuMaterial.AlbedoIndex = mBindlessMaterialArray.QueryIndex(material->AlbedoMap);
			gpuMaterial.NormalIndex = mBindlessMaterialArray.QueryIndex(material->NormalMap);
			gpuMaterial.MetalnessIndex = mBindlessMaterialArray.QueryIndex(material->MetalnessMap);
			gpuMaterial.RoughnessIndex = mBindlessMaterialArray.QueryIndex(material->RoughnessMap);
			gpuMaterial.AOIndex = mBindlessMaterialArray.QueryIndex(material->AOMap);
			gpuMaterial.EmissiveIndex = mBindlessMaterialArray.QueryIndex(material->EmissiveMap);

			mBindlessMaterialArray.Update(index, gpuMaterial);
		}
		else
		{
			SPDLOG_INFO("Material does not exist in bindless array");
		}
	}

	void SceneRenderer::RemoveTexture(WeakRef<Texture> texture)
	{
		mBindlessTextureArray.Remove(texture->Handle());
	}

	void SceneRenderer::RemoveMaterial(WeakRef<Material> material)
	{
		mBindlessMaterialArray.Remove(material->Handle());
	}
}