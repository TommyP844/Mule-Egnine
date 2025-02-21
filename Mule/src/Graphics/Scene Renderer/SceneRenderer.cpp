#include "Graphics/Scene Renderer/SceneRenderer.h"

#include "Graphics/VertexLayout.h"
#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"
#include "ECS/Components.h"
#include "Asset/Loader/GraphicsShaderLoader.h"
#include "Graphics/RenderTypes.h"

#include "Engine Context/EngineAssets.h"

namespace Mule
{
	SceneRenderer::SceneRenderer(Ref<GraphicsContext> context, Ref<AssetManager> assetManager)
		:
		mGraphicsContext(context),
		mAssetManager(assetManager),
		mFrameIndex(0)
	{
		DescriptorSetLayoutDescription BindlessTextureDSLD{
			.Layouts = {
				LayoutDescription(0, DescriptorType::Texture, ShaderStage::Fragment, 4096)
			}
		};
		mBindlessTextureDSL = mGraphicsContext->CreateDescriptorSetLayout(BindlessTextureDSLD);

		DescriptorSetDescription BindlessTextureDSD{
			.Layouts = {
				mBindlessTextureDSL
			}
		};

		mFrameData.resize(2);
		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].BindlessTextureDS = mGraphicsContext->CreateDescriptorSet(BindlessTextureDSD);

			mFrameData[i].CameraBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPU::GPUCamera));
			mFrameData[i].MaterialBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPU::GPUMaterial) * 1000);
			mFrameData[i].LightBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPU::GPULightData));
		}

		GeometryPassInitInfo geometryInitInfo{
			.AssetManager = mAssetManager,
			.MaterialArray = mMaterialArray,
			.TextureArray = mTextureArray
		};

		geometryInitInfo.FrameInfo = {
			{
				mFrameData[0].CameraBuffer,
				mFrameData[0].MaterialBuffer,
				mFrameData[0].LightBuffer,
				mFrameData[0].BindlessTextureDS
			},
			{
				mFrameData[1].CameraBuffer,
				mFrameData[1].MaterialBuffer,
				mFrameData[1].LightBuffer,
				mFrameData[1].BindlessTextureDS
			}
		};

		mGeometryPass = MakeRef<GeometryPass>(mGraphicsContext, geometryInitInfo);
	}
	
	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::RefreshEngineObjects()
	{
		auto blackTexture = mAssetManager->GetAsset<Texture2D>(MULE_BLACK_TEXTURE_HANDLE);
		mBlackImageIndex = mTextureArray.Insert(blackTexture->Handle(), blackTexture);

		auto blackCubeMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);
		mBlackCubeMapImageIndex = mTextureArray.Insert(blackCubeMap->Handle(), blackCubeMap);

		auto whiteTexture = mAssetManager->GetAsset<Texture2D>(MULE_WHITE_TEXTURE_HANDLE);
		mWhiteImageIndex = mTextureArray.Insert(whiteTexture->Handle(), whiteTexture);
	}

	void SceneRenderer::AddTexture(WeakRef<ITexture> texture)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		uint32_t index = mTextureArray.Insert(texture->Handle(), texture);
		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].AddedTextures.push_back({ texture, index });
		}
	}

	void SceneRenderer::RemoveTexture(AssetHandle textureHandle)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mTextureArray.Remove(textureHandle);
	}

	void SceneRenderer::AddMaterial(WeakRef<Material> material)
	{
		GPU::GPUMaterial gpuMaterial{};

		gpuMaterial.AlbedoColor = material->AlbedoColor;

		gpuMaterial.TextureScale = material->TextureScale;

		gpuMaterial.MetalnessFactor = material->MetalnessFactor;
		gpuMaterial.RoughnessFactor = material->RoughnessFactor;
		gpuMaterial.AOFactor = material->AOFactor;

		gpuMaterial.AlbedoIndex = mTextureArray.QueryIndex(material->AlbedoMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AlbedoMap);
		gpuMaterial.NormalIndex = mTextureArray.QueryIndex(material->NormalMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->NormalMap);
		gpuMaterial.MetalnessIndex = mTextureArray.QueryIndex(material->MetalnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->MetalnessMap);
		gpuMaterial.RoughnessIndex = mTextureArray.QueryIndex(material->RoughnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->RoughnessMap);
		gpuMaterial.AOIndex = mTextureArray.QueryIndex(material->AOMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AOMap);
		gpuMaterial.EmissiveIndex = mTextureArray.QueryIndex(material->EmissiveMap) == UINT32_MAX ? mBlackImageIndex : mTextureArray.QueryIndex(material->EmissiveMap);

		std::lock_guard<std::mutex> lock(mMutex);
		uint32_t index = mMaterialArray.Insert(material->Handle(), gpuMaterial);

		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].AddedMaterials.push_back({ gpuMaterial, index });
		}
	}

	void SceneRenderer::UpdateMaterial(WeakRef<Material> material)
	{
		GPU::GPUMaterial gpuMaterial{};

		gpuMaterial.AlbedoColor = material->AlbedoColor;

		gpuMaterial.TextureScale = material->TextureScale;

		gpuMaterial.MetalnessFactor = material->MetalnessFactor;
		gpuMaterial.RoughnessFactor = material->RoughnessFactor;
		gpuMaterial.AOFactor = material->AOFactor;

		gpuMaterial.AlbedoIndex = mTextureArray.QueryIndex(material->AlbedoMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AlbedoMap);
		gpuMaterial.NormalIndex = mTextureArray.QueryIndex(material->NormalMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->NormalMap);
		gpuMaterial.MetalnessIndex = mTextureArray.QueryIndex(material->MetalnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->MetalnessMap);
		gpuMaterial.RoughnessIndex = mTextureArray.QueryIndex(material->RoughnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->RoughnessMap);
		gpuMaterial.AOIndex = mTextureArray.QueryIndex(material->AOMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AOMap);
		gpuMaterial.EmissiveIndex = mTextureArray.QueryIndex(material->EmissiveMap) == UINT32_MAX ? mBlackImageIndex : mTextureArray.QueryIndex(material->EmissiveMap);

		std::lock_guard<std::mutex> lock(mMutex);
		uint32_t index = mMaterialArray.QueryIndex(material->Handle());

		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].AddedMaterials.push_back({ gpuMaterial, index });
		}
	}

	void SceneRenderer::RemoveMaterial(AssetHandle materialHandle)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mMaterialArray.Remove(materialHandle);
	}

	void SceneRenderer::OnRender(WeakRef<Scene> scene, std::vector<WeakRef<Semaphore>> waitSemaphore)
	{

	}
	
	void SceneRenderer::OnEditorRender(const EditorRenderSettings& settings)
	{
		mFrameIndex ^= 1;
		FrameData& frameData = mFrameData[mFrameIndex];
		mGeometryPass->NextFrame();

		// TODO: revisit this: this only works because when the geometry fence passes it prettyt much garuntees shadows are finished
		mGeometryPass->GetFence()->Wait();
		mGeometryPass->GetFence()->Reset();

		if (frameData.ResizeRequired)
		{
			frameData.ResizeRequired = false;
			mGeometryPass->Resize(frameData.ResizeWidth, frameData.ResizeHeight);
		}

		PrepDrawData(settings.Scene, settings.EditorCamera);

		// Scene Render Passes
		std::vector<WeakRef<Semaphore>> semaphores;

		GeometryPassRenderInfo geometryRenderInfo{};
		geometryRenderInfo.Scene = settings.Scene;
		geometryRenderInfo.WaitSemaphores = semaphores;
		geometryRenderInfo.RenderShadows = false;
		geometryRenderInfo.Camera = settings.EditorCamera;
		geometryRenderInfo.SelectedEntity = settings.SelectedEntities.empty() ? Entity() : settings.SelectedEntities[0];

		mGeometryPass->Render(geometryRenderInfo);

	}

	void SceneRenderer::Resize(uint32_t width, uint32_t height)
	{
		for (int i = 0; i < mFrameData.size(); i++)
		{
			mFrameData[i].ResizeRequired = true;
			mFrameData[i].ResizeWidth = width;
			mFrameData[i].ResizeHeight = height;
		}
	}

	void SceneRenderer::PrepDrawData(WeakRef<Scene> scene, const Camera& camera)
	{
		FrameData& frameData = mFrameData[mFrameIndex];

		// Camera
		{
			GPU::GPUCamera cameraData{};
			cameraData.View = camera.GetView();
			cameraData.Proj = camera.GetProj();
			cameraData.CameraPos = camera.GetPosition();
			frameData.CameraBuffer->SetData(&cameraData, sizeof(cameraData));
		}

		// Lights
		{
			frameData.LightData.DirectionalLight.Color = glm::vec3(0.f);
			frameData.LightData.DirectionalLight.Direction = glm::vec3(0.f);
			frameData.LightData.DirectionalLight.Intensity = 0.f;
			frameData.LightData.NumPointLights = 0;

			for (auto entityId : scene->Iterate<DirectionalLightComponent>())
			{
				Entity e((uint32_t)entityId, scene);
				DirectionalLightComponent& directionalLight = e.GetComponent<DirectionalLightComponent>();
				if (!directionalLight.Active)
					continue;

				frameData.LightData.DirectionalLight.Color = directionalLight.Color;
				frameData.LightData.DirectionalLight.Intensity = directionalLight.Intensity;
				glm::quat rotation = glm::quat(glm::radians(e.GetTransformComponent().Rotation));
				glm::vec4 direction = rotation * glm::vec4(0, -1, 0, 0);
				frameData.LightData.DirectionalLight.Direction = glm::normalize(direction);

				break;
			}

			for (auto entityId : scene->Iterate<PointLightComponent>())
			{
				Entity e((uint32_t)entityId, scene);

				PointLightComponent& pointLight = e.GetComponent<PointLightComponent>();

				if (!pointLight.Active)
					continue;

				frameData.LightData.PointLights[frameData.LightData.NumPointLights].Color = pointLight.Color;
				frameData.LightData.PointLights[frameData.LightData.NumPointLights].Intensity = pointLight.Radiance;
				frameData.LightData.PointLights[frameData.LightData.NumPointLights++].Position = e.GetTransformComponent().Translation;
			}

			frameData.LightBuffer->SetData(&frameData.LightData, sizeof(GPU::GPULightData));
		}

		// Materials
		{
			for (auto& [gpuMaterial, index] : frameData.AddedMaterials)
			{
				frameData.MaterialBuffer->SetData(&gpuMaterial, sizeof(GPU::GPUMaterial), index * sizeof(GPU::GPUMaterial));
			}
			frameData.AddedMaterials.clear();
		}

		// Textures
		{
			std::vector<DescriptorSetUpdate> updates;
			for (auto& [texture, index] : frameData.AddedTextures)
			{
				DescriptorSetUpdate update{};
				update.Binding = 0;
				update.ArrayElement = index;
				update.Textures = { texture };
				update.Type = DescriptorType::Texture;
				updates.emplace_back(update);
			}
			frameData.BindlessTextureDS->Update(updates);
			frameData.AddedTextures.clear();
		}
	}
}