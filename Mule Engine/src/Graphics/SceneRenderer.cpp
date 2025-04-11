#include "Graphics/SceneRenderer.h"

#include "Graphics/VertexLayout.h"
#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/CommandPool.h"

#include "Asset/Serializer/GraphicsShaderSerializer.h"
#include "ECS/Components.h"
#include "Timer.h"

#include "Engine Context/EngineAssets.h"


namespace Mule
{
	SceneRenderer::SceneRenderer(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager)
		:
		mGraphicsContext(context),
		mAssetManager(assetManager)
	{
		mGraph = RenderGraph::RenderGraph(context, assetManager);
		mResourceUpdates.resize(mGraph.GetFrameCount());
		mTiming.resize(mGraph.GetFrameCount());

#pragma region Create Resources

		// Uniform Buffers
		{
			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				auto cameraBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPU::GPUCamera));
				mGraph.AddResource(i, CAMERA_BUFFER_ID, cameraBuffer);

				auto lightBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPU::GPULightData));
				mGraph.AddResource(i, LIGHT_BUFFER_ID, lightBuffer);

				auto materialBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPU::GPUMaterial) * 800);
				mGraph.AddResource(i, MATERIAL_BUFFER_ID, materialBuffer);
			}
		}

		// Descriptor Set Layouts
		{			
			auto geometryShaderDSL = mGraphicsContext->CreateDescriptorSetLayout({
				LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Vertex | ShaderStage::Fragment), // Camera Buffer
				LayoutDescription(1, DescriptorType::UniformBuffer, ShaderStage::Fragment), // Material Buffer
				LayoutDescription(2, DescriptorType::UniformBuffer, ShaderStage::Fragment), // Light Buffer
				LayoutDescription(3, DescriptorType::Texture, ShaderStage::Fragment), // Light Buffer
				LayoutDescription(4, DescriptorType::Texture, ShaderStage::Fragment), // Light Buffer
				LayoutDescription(5, DescriptorType::Texture, ShaderStage::Fragment), // Light Buffer
				});

			auto bindlessTextureDSL = mGraphicsContext->CreateDescriptorSetLayout({
				LayoutDescription(0, DescriptorType::Texture, ShaderStage::Fragment, 4096)
				});

			auto environmentShaderDSL = mGraphicsContext->CreateDescriptorSetLayout({
				LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Vertex),
				LayoutDescription(1, DescriptorType::Texture, ShaderStage::Fragment)
				});

			auto entityOutlineDSL = mGraphicsContext->CreateDescriptorSetLayout({
				LayoutDescription(0, DescriptorType::StorageImage, ShaderStage::Compute),
				LayoutDescription(1, DescriptorType::Texture, ShaderStage::Compute),
				});

			auto entityMaskDSL = mGraphicsContext->CreateDescriptorSetLayout({
				LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Vertex), // Camera Buffer
				});

			auto billboardDSL = mGraphicsContext->CreateDescriptorSetLayout({
				LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Vertex), // Camera Buffer
				});

			auto wireframeDSL = mGraphicsContext->CreateDescriptorSetLayout({
				LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Vertex), // Camera Buffer
				});

			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				mGraph.AddResource(i, GEOMETRY_SHADER_DSL_ID, geometryShaderDSL);
				mGraph.AddResource(i, BINDLESS_TEXTURE_DSL_ID, bindlessTextureDSL);
				mGraph.AddResource(i, ENVIRONMENT_SHADER_DSL_ID, environmentShaderDSL);
				mGraph.AddResource(i, ENTITY_OUTLINE_DSL_ID, entityOutlineDSL);
				mGraph.AddResource(i, ENTITY_MASK_DSL_ID, entityMaskDSL);
				mGraph.AddResource(i, BILLBOARD_DSL_ID, billboardDSL);
				mGraph.AddResource(i, WIREFRAME_DSL_ID, wireframeDSL);
			}
		}

		// Descriptor Sets
		{
			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				// Geometry
				auto geometryShaderDS = mGraphicsContext->CreateDescriptorSet({
					mGraph.GetResource<DescriptorSetLayout>(i, GEOMETRY_SHADER_DSL_ID)
					});

				geometryShaderDS->Update({
					DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) }),
					DescriptorSetUpdate(1, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, MATERIAL_BUFFER_ID) }),
					DescriptorSetUpdate(2, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, LIGHT_BUFFER_ID) })
					});

				mGraph.AddResource(i, GEOMETRY_SHADER_DS_ID, geometryShaderDS);


				// Bindless textures
				auto bindlessTextureDS = mGraphicsContext->CreateDescriptorSet({
					mGraph.GetResource<DescriptorSetLayout>(i, BINDLESS_TEXTURE_DSL_ID)
					});
				mGraph.AddResource(i, BINDLESS_TEXTURE_DS_ID, bindlessTextureDS);


				// Environment map
				auto environmentShaderDS = mGraphicsContext->CreateDescriptorSet({
					mGraph.GetResource<DescriptorSetLayout>(i, ENVIRONMENT_SHADER_DSL_ID),
					});
				environmentShaderDS->Update({
					DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) })
					});
				mGraph.AddResource(i, ENVIRONMENT_SHADER_DS_ID, environmentShaderDS);


				// outline 
				auto entityOutlineDS = mGraphicsContext->CreateDescriptorSet({
					mGraph.GetResource<DescriptorSetLayout>(i, ENTITY_OUTLINE_DSL_ID)
					});
				mGraph.AddResource(i, ENTITY_OUTLINE_DS_ID, entityOutlineDS);


				// mask
				auto entityMaskDS = mGraphicsContext->CreateDescriptorSet({
					mGraph.GetResource<DescriptorSet>(i, ENTITY_MASK_DSL_ID)
					});
				entityMaskDS->Update({
					DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) })
					});
				mGraph.AddResource(i, ENTITY_MASK_DS_ID, entityMaskDS);


				// billboard
				auto billboardDS = mGraphicsContext->CreateDescriptorSet({
					mGraph.GetResource<DescriptorSet>(i, BILLBOARD_DSL_ID)
					});
				billboardDS->Update({
					DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) })
					});
				mGraph.AddResource(i, BILLBOARD_DS_ID, billboardDS);

				// billboard
				auto wireframeDS = mGraphicsContext->CreateDescriptorSet({
					mGraph.GetResource<DescriptorSet>(i, WIREFRAME_DSL_ID)
					});
				wireframeDS->Update({
					DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) })
					});
				mGraph.AddResource(i, WIREFRAME_DS_ID, wireframeDS);
			}
		}

#pragma endregion

#pragma region Render Graph Init

		// Framebuffers
		FramebufferDescription framebufferDesc;
		framebufferDesc.Attachments = {
			{ TextureFormat::RGBA32F, TextureFlags::StorageImage },
			{ TextureFormat::RG32UI, TextureFlags::None },
			{ TextureFormat::R32F, TextureFlags::None }
		};
		framebufferDesc.DepthAttachment = { TextureFormat::D32F, TextureFlags::None };
		framebufferDesc.Height = 600;
		framebufferDesc.Width = 800;
		framebufferDesc.LayerCount = 1;
		
		mGraph.CreateFramebuffer(framebufferDesc);

		mGraph.SetFinalLayouts({ { 0, ImageLayout::ShaderReadOnly } });
				
		mGraph.AddPass(
			ENVIRONMENT_PASS_NAME,
			{ },
			MULE_ENVIRONMENT_MAP_SHADER_HANDLE,
			{ { 0, ImageLayout::ColorAttachment } },
			std::bind(&SceneRenderer::RenderEnvironmentCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

		mGraph.AddPass(
			GEOMETRY_PASS_NAME,
			{ ENVIRONMENT_PASS_NAME },
			MULE_GEOMETRY_SHADER_HANDLE,
			{ 
				{ 0, ImageLayout::ColorAttachment },
				{ 1, ImageLayout::ColorAttachment }
			},
			std::bind(&SceneRenderer::RenderSolidGeometryCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

		mGraph.AddPass(
			TRANPARENT_GEOMETRY_PASS_NAME,
			{ GEOMETRY_PASS_NAME },
			MULE_TRANSPARENT_SHADER_HANDLE,
			{
				{ 0, ImageLayout::ColorAttachment },
				{ 1, ImageLayout::ColorAttachment }
			},
			std::bind(&SceneRenderer::RenderTransparentGeometryCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

		mGraph.AddPass(
			ENTITY_HIGHLIGHT_PASS_NAME,
			{ TRANPARENT_GEOMETRY_PASS_NAME },
			MULE_ENTITY_MASK_SHADER_HANDLE,
			{ { 2, ImageLayout::ColorAttachment } },
			std::bind(&SceneRenderer::RenderEntityMaskCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

		mGraph.AddPass(
			ENTTIY_OUTLINE_PASS_NAME,
			{ ENTITY_HIGHLIGHT_PASS_NAME },
			AssetHandle::Null(),
			{
				{ 0, ImageLayout::General},
				{ 2, ImageLayout::ShaderReadOnly }
			},
			std::bind(&SceneRenderer::RenderEntityOutlineCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

		mGraph.AddPass(
			EDITOR_BILLBOARD_PASS_NAME,
			{ ENTTIY_OUTLINE_PASS_NAME },
			MULE_BILLBOARD_SHADER_HANDLE,
			{ { 0, ImageLayout::ColorAttachment } },
			std::bind(&SceneRenderer::RenderEditorBillboardCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);		
		
		mGraph.AddPass(
			PHYSICS_UI_PASS_NAME,
			{ EDITOR_BILLBOARD_PASS_NAME },
			MULE_WIRE_FRAME_SHADER_HANDLE,
			{ { 0, ImageLayout::ColorAttachment } },
			std::bind(&SceneRenderer::RenderPhysicsUIPass, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

		mGraph.Compile();

		// Force resize so that way we can bind framebuffers images to the entity outline descriptor set
		// with proper layout transitions
		for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
		{
			mResourceUpdates[i].Resize = true;
			mResourceUpdates[i].ResizeWidth = 800;
			mResourceUpdates[i].ResizeHeight = 600;
		}
#pragma endregion
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
		for (int i = 0; i < mGraph.GetFrameCount(); i++)
		{
			mResourceUpdates[i].TextureUpdates.push_back({ texture, index });
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
		gpuMaterial.Transparency = material->Transparent ? material->Transparency : 1.0;

		uint32_t albedoIndex = mTextureArray.QueryIndex(material->AlbedoMap);
		if (albedoIndex == UINT32_MAX)
		{
			if (material->AlbedoMap)
			{
				gpuMaterial.AlbedoIndex = mTextureArray.Insert(material->AlbedoMap, nullptr);
			}
			else
			{
				gpuMaterial.AlbedoIndex = mWhiteImageIndex;
			}
		}
		else
		{
			gpuMaterial.AlbedoIndex = albedoIndex;
		}

		gpuMaterial.AlbedoIndex = QueryOrInsertTextureIndex(material->AlbedoMap, mWhiteImageIndex);
		gpuMaterial.NormalIndex = QueryOrInsertTextureIndex(material->NormalMap, UINT32_MAX);
		gpuMaterial.MetalnessIndex = QueryOrInsertTextureIndex(material->MetalnessMap, mWhiteImageIndex);
		gpuMaterial.RoughnessIndex = QueryOrInsertTextureIndex(material->RoughnessMap, mWhiteImageIndex);
		gpuMaterial.AOIndex = QueryOrInsertTextureIndex(material->AOMap, mWhiteImageIndex);
		gpuMaterial.EmissiveIndex = QueryOrInsertTextureIndex(material->EmissiveMap, mBlackImageIndex);
		gpuMaterial.OpacityIndex = QueryOrInsertTextureIndex(material->OpacityMap, UINT32_MAX);

		std::lock_guard<std::mutex> lock(mMutex);
		uint32_t index = mMaterialArray.Insert(material->Handle(), gpuMaterial);

		for (int i = 0; i < mGraph.GetFrameCount(); i++)
		{
			mResourceUpdates[i].MaterialUpdates.push_back({ gpuMaterial, index });
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
		gpuMaterial.Transparency = material->Transparent ? material->Transparency : 1.0;

		gpuMaterial.AlbedoIndex = mTextureArray.QueryIndex(material->AlbedoMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AlbedoMap);
		gpuMaterial.NormalIndex = mTextureArray.QueryIndex(material->NormalMap) == UINT32_MAX ? -1 : mTextureArray.QueryIndex(material->NormalMap);
		gpuMaterial.MetalnessIndex = mTextureArray.QueryIndex(material->MetalnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->MetalnessMap);
		gpuMaterial.RoughnessIndex = mTextureArray.QueryIndex(material->RoughnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->RoughnessMap);
		gpuMaterial.AOIndex = mTextureArray.QueryIndex(material->AOMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AOMap);
		gpuMaterial.EmissiveIndex = mTextureArray.QueryIndex(material->EmissiveMap) == UINT32_MAX ? mBlackImageIndex : mTextureArray.QueryIndex(material->EmissiveMap);
		gpuMaterial.OpacityIndex = mTextureArray.QueryIndex(material->OpacityMap) == UINT32_MAX ? -1 : mTextureArray.QueryIndex(material->OpacityMap);

		std::lock_guard<std::mutex> lock(mMutex);
		uint32_t index = mMaterialArray.QueryIndex(material->Handle());
		mMaterialArray.Update(index, gpuMaterial);

		for (int i = 0; i < mGraph.GetFrameCount(); i++)
		{
			mResourceUpdates[i].MaterialUpdates.push_back({ gpuMaterial, index });
		}
	}

	void SceneRenderer::RemoveMaterial(AssetHandle materialHandle)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mMaterialArray.Remove(materialHandle);
	}

	bool SceneRenderer::OnEditorRender(WeakRef<Scene> scene, const Camera& camera, const std::vector<WeakRef<Semaphore>>& waitSemaphores)
	{
		auto& timingInfo = mTiming[mGraph.GetFrameIndex()];
		// We call next frame first so all graph queries after render will reflect the currect frame
		mGraph.NextFrame();
		mGraph.Wait();
		
		mGraph.SetCamera(camera);

		PrepareResources(camera, scene);

		Timer executionTimer;
		executionTimer.Start();
		
		mGraph.Execute(scene, waitSemaphores);
		
		executionTimer.Stop();
		timingInfo.CPUExecutionTime = executionTimer.Query();
		timingInfo.RenderPassStats = mGraph.GetRenderPassStats();

		return true;
	}

	bool SceneRenderer::OnRender(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores)
	{
		Ref<Camera> camera = nullptr;
		for (auto entity : scene->Iterate<CameraComponent>())
		{
			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			if (!cameraComponent.Active)
				continue;
			camera = cameraComponent.Camera;
			break;
		}

		if (!camera)
		{
			return false;
		}

		auto& timingInfo = mTiming[mGraph.GetFrameIndex()];
		// We call next frame first so all graph queries after render will reflect the currect frame
		mGraph.NextFrame();
		mGraph.Wait();

		mGraph.SetCamera(*camera);

		PrepareResources(*camera, scene);

		Timer executionTimer;
		executionTimer.Start();

		mGraph.Execute(scene, waitSemaphores);

		executionTimer.Stop();
		timingInfo.CPUExecutionTime = executionTimer.Query();
		timingInfo.RenderPassStats = mGraph.GetRenderPassStats();

		return true;
	}

	Guid SceneRenderer::Pick(uint32_t x, uint32_t y)
	{
		mGraph.Wait();
		uint32_t index = mGraph.GetFrameIndex();
		auto framebuffer = mGraph.GetCurrentFrameBuffer();

		auto queue = mGraphicsContext->GetGraphicsQueue();
		auto cmdPool = queue->CreateCommandPool();
		auto cmd = cmdPool->CreateCommandBuffer();
		auto fence = mGraphicsContext->CreateFence();
		auto stagingBuffer = mGraphicsContext->CreateStagingBuffer(2 * sizeof(uint32_t));
		
		fence->Wait();
		fence->Reset();
		cmd->Begin();
		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(1), ImageLayout::TransferSrc);
		cmd->ReadTexture(framebuffer->GetColorAttachment(1), x, y, 1, 1, stagingBuffer);
		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(1), ImageLayout::ShaderReadOnly);

		cmd->End();
		queue->Submit({ cmd }, {}, {}, fence);
		fence->Wait();
		fence->Reset();
		
		// Get Results

		Buffer buffer = stagingBuffer->ReadData();

		uint64_t guidHigh = buffer.As<uint32_t>()[0];
		uint64_t guidLow = buffer.As<uint32_t>()[1];
		uint64_t data = guidHigh << 32 | guidLow;

		return Guid(data);
	}

	WeakRef<Semaphore> SceneRenderer::GetSemaphore() const
	{
		return mGraph.GetSemaphore();
	}

	WeakRef<FrameBuffer> SceneRenderer::GetFrameBuffer() const
	{
		return mGraph.GetCurrentFrameBuffer();
	}
	
	void SceneRenderer::Resize(uint32_t width, uint32_t height)
	{
		for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
		{
			mResourceUpdates[i].Resize = true;
			mResourceUpdates[i].ResizeWidth = width;
			mResourceUpdates[i].ResizeHeight = height;
		}
	}

	uint32_t SceneRenderer::QueryOrInsertTextureIndex(AssetHandle handle, uint32_t defaultIndex)
	{
		uint32_t index = mTextureArray.QueryIndex(handle);
		if (index == UINT32_MAX)
		{
			if (handle)
			{
				index = mTextureArray.Insert(handle, nullptr);
			}
			else
			{
				index = defaultIndex;
			}
		}

		return index;
	}

	void SceneRenderer::PrepareResources(const Camera& camera, WeakRef<Scene> scene)
	{
		auto& timingInfo = mTiming[mGraph.GetFrameIndex()];

		Timer dataPrepTimer;
		dataPrepTimer.Start();
		// Data Prep
		{
			WeakRef<UniformBuffer> cameraUB = mGraph.QueryResource<UniformBuffer>(CAMERA_BUFFER_ID);
			WeakRef<UniformBuffer> lightUB = mGraph.QueryResource<UniformBuffer>(LIGHT_BUFFER_ID);
			WeakRef<UniformBuffer> materialBuffer = mGraph.QueryResource<DescriptorSet>(MATERIAL_BUFFER_ID);
			WeakRef<DescriptorSet> bindlessTextureDS = mGraph.QueryResource<DescriptorSet>(BINDLESS_TEXTURE_DS_ID);

			// Camera
			{
				GPU::GPUCamera cameraData{};
				cameraData.View = camera.GetView();
				cameraData.Proj = camera.GetProj();
				cameraData.CameraPos = camera.GetPosition();
				cameraUB->SetData(&cameraData, sizeof(cameraData));
			}

			// Lights
			{
				GPU::GPULightData lightData;

				for (auto entity : scene->Iterate<DirectionalLightComponent>())
				{
					DirectionalLightComponent& directionalLight = entity.GetComponent<DirectionalLightComponent>();
					if (!directionalLight.Active)
						continue;

					lightData.DirectionalLight.Color = directionalLight.Color;
					lightData.DirectionalLight.Intensity = directionalLight.Intensity;
					glm::quat rotation = glm::quat(glm::radians(entity.GetTransformComponent().Rotation));
					glm::vec4 direction = rotation * glm::vec4(0, -1, 0, 0);
					lightData.DirectionalLight.Direction = glm::normalize(direction);

					break;
				}

				for (auto entity : scene->Iterate<PointLightComponent>())
				{
					PointLightComponent& pointLight = entity.GetComponent<PointLightComponent>();

					if (!pointLight.Active)
						continue;

					lightData.PointLights[lightData.NumPointLights].Color = pointLight.Color;
					lightData.PointLights[lightData.NumPointLights].Intensity = pointLight.Radiance;
					lightData.PointLights[lightData.NumPointLights++].Position = entity.GetTransformComponent().Translation;
				}

				lightUB->SetData(&lightData, sizeof(GPU::GPULightData));
			}

			auto& resourceUpdate = mResourceUpdates[mGraph.GetFrameIndex()];
			// Materials
			{
				for (auto& [gpuMaterial, index] : resourceUpdate.MaterialUpdates)
				{
					materialBuffer->SetData(&gpuMaterial, sizeof(GPU::GPUMaterial), index * sizeof(GPU::GPUMaterial));
				}
				resourceUpdate.MaterialUpdates.clear();
			}

			// Textures
			{
				std::vector<DescriptorSetUpdate> updates;
				for (auto& [texture, index] : resourceUpdate.TextureUpdates)
				{
					DescriptorSetUpdate update(0, DescriptorType::Texture, index, { texture }, {});
					updates.emplace_back(update);
				}
				bindlessTextureDS->Update(updates);
				resourceUpdate.TextureUpdates.clear();
			}

			if (resourceUpdate.Resize)
			{
				resourceUpdate.Resize = false;
				mGraph.Resize(resourceUpdate.ResizeWidth, resourceUpdate.ResizeHeight);
			}
		}
		dataPrepTimer.Stop();
		timingInfo.CPUPrepareTime = dataPrepTimer.Query();
	}

	void SceneRenderer::RenderSolidGeometryCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx)
	{
		WeakRef<DescriptorSet> geometryDS = ctx.Get<DescriptorSet>(GEOMETRY_SHADER_DS_ID);
		WeakRef<DescriptorSet> bindlessTextureDS = ctx.Get<DescriptorSet>(BINDLESS_TEXTURE_DS_ID);

		WeakRef<EnvironmentMap> environmentMap = nullptr;
		for (auto entity : scene->Iterate<EnvironmentMapComponent>())
		{
			const auto& envMapComponent = entity.GetComponent<EnvironmentMapComponent>();
			if (!envMapComponent.Active)
				continue;

			environmentMap = mAssetManager->GetAsset<EnvironmentMap>(envMapComponent.EnvironmentMap);

			break;
		}

		if (environmentMap)
		{
			auto irradianceMap = mAssetManager->GetAsset<TextureCube>(environmentMap->GetDiffuseIBLMap());
			auto prefilterMap = mAssetManager->GetAsset<TextureCube>(environmentMap->GetPreFilterMap());
			auto brdfLutMap = mAssetManager->GetAsset<Texture2D>(environmentMap->GetBRDFLutMap());
			std::vector<DescriptorSetUpdate> geometryShaderDSUs = {
				DescriptorSetUpdate(3, DescriptorType::Texture, 0, { irradianceMap }, {}),
				DescriptorSetUpdate(4, DescriptorType::Texture, 0, { prefilterMap }, {}),
				DescriptorSetUpdate(5, DescriptorType::Texture, 0, { brdfLutMap }, {}),
			};
			geometryDS->Update(geometryShaderDSUs);
		}
		else
		{
			auto blackCubeMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);
			auto blackTexture = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_HANDLE);
			std::vector<DescriptorSetUpdate> geometryShaderDSUs = {
				DescriptorSetUpdate(3, DescriptorType::Texture, 0, { blackCubeMap }, {}),
				DescriptorSetUpdate(4, DescriptorType::Texture, 0, { blackCubeMap }, {}),
				DescriptorSetUpdate(5, DescriptorType::Texture, 0, { blackTexture }, {}),
			};
			geometryDS->Update(geometryShaderDSUs);
		}

		cmd->BindGraphicsDescriptorSet(shader, { geometryDS, bindlessTextureDS });

		for (auto entity : scene->Iterate<MeshComponent>())
		{
			const MeshComponent& meshComponent = entity.GetComponent<MeshComponent>();
			if (!meshComponent.Visible) continue;

			auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
			if (!mesh) continue;

			uint32_t materialIndex = mMaterialArray.QueryIndex(meshComponent.MaterialHandle);
			if (materialIndex == UINT32_MAX)
				materialIndex = 0;

			const GPU::GPUMaterial& material = mMaterialArray.Query(materialIndex);
 			if (material.AlbedoColor.a < 1.0 || material.Transparency < 1.0 || material.OpacityIndex != -1)
			{
				continue;
			}

			glm::mat4 transform = entity.GetTransform();
			uint64_t guid = entity.Guid();

			// set push constants
			uint32_t fragmentConstants[] = {
				materialIndex,
				0, //padding
				(uint32_t)(guid >> 32u),
				(uint32_t)(guid & 0xFFFFFFFF)
			};

			cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
			cmd->SetPushConstants(shader, ShaderStage::Fragment, &fragmentConstants[0], sizeof(uint32_t) * 4);

			cmd->BindAndDrawMesh(mesh, 1);
		}
	}

	void SceneRenderer::RenderTransparentGeometryCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx)
	{
		WeakRef<DescriptorSet> geometryDS = ctx.Get<FrameBuffer>(GEOMETRY_SHADER_DS_ID);
		WeakRef<DescriptorSet> bindlessTextureDS = ctx.Get<FrameBuffer>(BINDLESS_TEXTURE_DS_ID);

		cmd->BindGraphicsDescriptorSet(shader, { geometryDS, bindlessTextureDS });

		for (auto entity : scene->Iterate<MeshComponent>())
		{
			const MeshComponent& meshComponent = entity.GetComponent<MeshComponent>();
			if (!meshComponent.Visible) continue;

			auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
			if (!mesh) continue;

			uint32_t materialIndex = mMaterialArray.QueryIndex(meshComponent.MaterialHandle);
			if (materialIndex == UINT32_MAX)
				materialIndex = 0;

			const GPU::GPUMaterial& material = mMaterialArray.Query(materialIndex);
			if (material.AlbedoColor.a == 1.0 && material.Transparency == 1.0 && material.OpacityIndex == -1)
			{
				continue;
			}

			glm::mat4 transform = entity.GetTransform();
			
			uint64_t guid = entity.Guid();

			// set push constants
			uint32_t fragmentConstants[] = {
				materialIndex,
				0, // padding
				guid >> 32,
				guid & 0xFFFFFFFF
			};

			cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
			cmd->SetPushConstants(shader, ShaderStage::Fragment, &fragmentConstants[0], sizeof(uint32_t) * 4);

			cmd->BindAndDrawMesh(mesh, 1);
		}
	}

	void SceneRenderer::RenderEnvironmentCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx)
	{
		WeakRef<DescriptorSet> DS = ctx.Get<DescriptorSet>(ENVIRONMENT_SHADER_DS_ID);
	
		if (!shader)
			return;

		WeakRef<TextureCube> cubeMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);
		for (auto entity : scene->Iterate<EnvironmentMapComponent>())
		{
			const auto& environmentMapComponent = entity.GetComponent<EnvironmentMapComponent>();
			if (!environmentMapComponent.Active)
				continue;

			auto environemntMap = mAssetManager->GetAsset<EnvironmentMap>(environmentMapComponent.EnvironmentMap);
			if (environemntMap)
			{
				if (environmentMapComponent.DisplayIrradianceMap)
				{
					cubeMap = mAssetManager->GetAsset<TextureCube>(environemntMap->GetDiffuseIBLMap());
				}
				else
				{
					cubeMap = mAssetManager->GetAsset<TextureCube>(environemntMap->GetCubeMapHandle());
				}
			}

			break;
		}

		DS->Update({
			DescriptorSetUpdate(1, DescriptorType::Texture, 0, { cubeMap }, {})
			});

		cmd->BindGraphicsDescriptorSet(shader, { DS });
		auto mesh = mAssetManager->GetAsset<Mesh>(MULE_CUBE_MESH_HANDLE);
		if (mesh)
		{
			cmd->BindAndDrawMesh(mesh, 1);
		}
	}

	void SceneRenderer::RenderEntityMaskCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx)
	{
		WeakRef<DescriptorSet> DS = ctx.Get<DescriptorSet>(ENTITY_MASK_DS_ID);
		cmd->BindGraphicsDescriptorSet(shader, { DS });

		std::function<void(Entity)> recurse = [&](Entity e) {
			if (e.HasComponent<MeshComponent>())
			{
				glm::mat4 transform = e.GetTransform();
				auto& meshComponent = e.GetComponent<MeshComponent>();

				auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);

				if (mesh)
				{
					cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}

			for (auto child : e.Children())
			{
				recurse(child);
			}
		};

		bool hasHighlight = false;
		for (auto entity : scene->Iterate<HighlightComponent>())
		{
			recurse(entity);
		}
	}

	void SceneRenderer::RenderEntityOutlineCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx)
	{	
		WeakRef<FrameBuffer> framebuffer = mGraph.GetCurrentFrameBuffer();
		WeakRef<ComputeShader> outlineShader = mAssetManager->GetAsset<ComputeShader>(MULE_ENTITY_HIGLIGHT_SHADER_HANDLE);
		WeakRef<DescriptorSet> outlineDS = ctx.Get<DescriptorSet>(ENTITY_OUTLINE_DS_ID);

		outlineDS->Update({
			DescriptorSetUpdate(0, DescriptorType::StorageImage, 0, { framebuffer->GetColorAttachment(0) }, {}),
			DescriptorSetUpdate(1, DescriptorType::Texture, 0, { framebuffer->GetColorAttachment(2) }, {})
			});

		if (!outlineShader)
			return;

		auto width = framebuffer->GetWidth();
		auto height = framebuffer->GetHeight();

		cmd->BindComputePipeline(outlineShader);
		cmd->BindComputeDescriptorSet(outlineShader, outlineDS);
		
		cmd->Execute(width / 16, height / 16, 1);
	}

	void SceneRenderer::RenderEditorBillboardCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx)
	{
		WeakRef<DescriptorSet> billboardDS = ctx.Get<DescriptorSet>(BILLBOARD_DS_ID);
		WeakRef<DescriptorSet> bindlessDS = ctx.Get<DescriptorSet>(BINDLESS_TEXTURE_DS_ID);
		
		cmd->BindGraphicsDescriptorSet(shader, { billboardDS, bindlessDS });

		if (mDebugOptions.ShowAllLights)
		{
			auto mesh = mAssetManager->GetAsset<Mesh>(MULE_PLANE_MESH_HANDLE);

			for (auto entity : scene->Iterate<SpotLightComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();
				uint32_t textureIndex = mTextureArray.QueryIndex(MULE_SPOT_LIGHT_ICON_TEXTURE_HANDLE);

				if (mesh && textureIndex != UINT32_MAX)
				{
					glm::vec4 data[] = {
						transform[3],
						glm::vec4(2, 2, 0, 0)
					};

					cmd->SetPushConstants(shader, ShaderStage::Vertex, &data[0][0], 2 * sizeof(glm::vec4));
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &textureIndex, sizeof(uint32_t));
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}

			for (auto entity : scene->Iterate<PointLightComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();

				uint32_t textureIndex = mTextureArray.QueryIndex(MULE_POINT_LIGHT_ICON_TEXTURE_HANDLE);

				if (mesh && textureIndex != UINT32_MAX)
				{
					glm::vec4 data[] = {
						transform[3],
						glm::vec4(2, 2, 0, 0)
					};

					cmd->SetPushConstants(shader, ShaderStage::Vertex, &data[0][0], 2 * sizeof(glm::vec4));
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &textureIndex, sizeof(uint32_t));
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}
		}

		if (mDebugOptions.SelectedEntity)
		{
			Entity entity = mDebugOptions.SelectedEntity;
			glm::mat4 entityTransform = entity.GetTransformTR();

			if (mDebugOptions.ShowSelectedEntityLights && !mDebugOptions.ShowAllLights)
			{
				glm::mat4 transform = mDebugOptions.SelectedEntity.GetTransformTR();
				auto mesh = mAssetManager->GetAsset<Mesh>(MULE_PLANE_MESH_HANDLE);

				if (mDebugOptions.SelectedEntity.HasComponent<PointLightComponent>())
				{
					uint32_t textureIndex = mTextureArray.QueryIndex(MULE_POINT_LIGHT_ICON_TEXTURE_HANDLE);

					if (mesh && textureIndex != UINT32_MAX)
					{
						glm::vec4 data[] = {
							transform[3],
							glm::vec4(2, 2, 0, 0)
						};

						cmd->SetPushConstants(shader, ShaderStage::Vertex, &data[0][0], 2 * sizeof(glm::vec4));
						cmd->SetPushConstants(shader, ShaderStage::Fragment, &textureIndex, sizeof(uint32_t));
						cmd->BindAndDrawMesh(mesh, 1);
					}
				}
				if (mDebugOptions.SelectedEntity.HasComponent<SpotLightComponent>())
				{
					uint32_t textureIndex = mTextureArray.QueryIndex(MULE_SPOT_LIGHT_ICON_TEXTURE_HANDLE);

					if (mesh && textureIndex != UINT32_MAX)
					{
						glm::vec4 data[] = {
							transform[3],
							glm::vec4(2, 2, 0, 0)
						};

						cmd->SetPushConstants(shader, ShaderStage::Vertex, &data[0][0], 2 * sizeof(glm::vec4));
						cmd->SetPushConstants(shader, ShaderStage::Fragment, &textureIndex, sizeof(uint32_t));
						cmd->BindAndDrawMesh(mesh, 1);
					}
				}
			}
		}
	}
	
	void SceneRenderer::RenderPhysicsUIPass(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx)
	{
		WeakRef<DescriptorSet> DS = ctx.Get<DescriptorSet>(WIREFRAME_DS_ID);

		cmd->BindGraphicsDescriptorSet(shader, { DS });

		const glm::vec3 triggerColor = { 0, 0.f, 1.f };
		const glm::vec3 colliderColor = { 1.f, 0.f, 0.f };

		if (mDebugOptions.ShowAllPhysicsObjects)
		{
			for (auto entity : scene->Iterate<BoxColliderComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();
				auto& collider = entity.GetComponent<BoxColliderComponent>();

				glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), collider.Offset);
				offsetMat = glm::scale(offsetMat, collider.Extent);
				transform = transform * offsetMat;

				// Draw box
				cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				if (collider.Trigger)
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &triggerColor[0], sizeof(glm::vec3));
				else
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &colliderColor[0], sizeof(glm::vec3));
				auto mesh = mAssetManager->GetAsset<Mesh>(MULE_CUBE_MESH_HANDLE);
				if (mesh)
				{
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}

			for (auto entity : scene->Iterate<SphereColliderComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();
				auto& collider = entity.GetComponent<SphereColliderComponent>();

				glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), collider.Offset);
				offsetMat = glm::scale(offsetMat, glm::vec3(collider.Radius));
				transform = transform * offsetMat;

				// Draw Sphere
				cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				if (collider.Trigger)
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &triggerColor[0], sizeof(glm::vec3));
				else
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &colliderColor[0], sizeof(glm::vec3));
				auto mesh = mAssetManager->GetAsset<Mesh>(MULE_SPHERE_MESH_HANDLE);
				if (mesh)
				{
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}

			for (auto entity : scene->Iterate<CapsuleColliderComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();
				auto& collider = entity.GetComponent<CapsuleColliderComponent>();
				glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), collider.Offset);
				float radius = collider.Radius;
				float halfHeight = collider.HalfHeight;
				offsetMat = glm::scale(offsetMat, glm::vec3(radius, halfHeight, radius));
				transform = transform * offsetMat;
				cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				if (collider.Trigger)
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &triggerColor[0], sizeof(glm::vec3));
				else
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &colliderColor[0], sizeof(glm::vec3));
				auto mesh = mAssetManager->GetAsset<Mesh>(MULE_CAPSULE_MESH_HANDLE);
				if (mesh)
				{
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}
		}

		if (mDebugOptions.SelectedEntity && !mDebugOptions.ShowAllPhysicsObjects)
		{
			auto entity = mDebugOptions.SelectedEntity;
			glm::mat4 transform = entity.GetTransformTR();

			if (entity.HasComponent<SphereColliderComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();
				auto& collider = entity.GetComponent<SphereColliderComponent>();

				glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), collider.Offset);
				offsetMat = glm::scale(offsetMat, glm::vec3(collider.Radius));
				transform = transform * offsetMat;

				// Draw Sphere
				cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				if (collider.Trigger)
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &triggerColor[0], sizeof(glm::vec3));
				else
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &colliderColor[0], sizeof(glm::vec3));
				auto mesh = mAssetManager->GetAsset<Mesh>(MULE_SPHERE_MESH_HANDLE);
				if (mesh)
				{
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}

			if (entity.HasComponent<BoxColliderComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();
				auto& collider = entity.GetComponent<BoxColliderComponent>();

				glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), collider.Offset);
				offsetMat = glm::scale(offsetMat, collider.Extent);
				transform = transform * offsetMat;

				// Draw box
				cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				if (collider.Trigger)
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &triggerColor[0], sizeof(glm::vec3));
				else
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &colliderColor[0], sizeof(glm::vec3));
				auto mesh = mAssetManager->GetAsset<Mesh>(MULE_CUBE_MESH_HANDLE);
				if (mesh)
				{
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}

			if (entity.HasComponent<CapsuleColliderComponent>())
			{
				glm::mat4 transform = entity.GetTransformTR();
				auto& collider = entity.GetComponent<CapsuleColliderComponent>();
				glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), collider.Offset);
				float radius = collider.Radius;
				float halfHeight = collider.HalfHeight;
				offsetMat = glm::scale(offsetMat, glm::vec3(radius, halfHeight, radius));
				transform = transform * offsetMat;
				cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				if (collider.Trigger)
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &triggerColor[0], sizeof(glm::vec3));
				else
					cmd->SetPushConstants(shader, ShaderStage::Fragment, &colliderColor[0], sizeof(glm::vec3));
				auto mesh = mAssetManager->GetAsset<Mesh>(MULE_CAPSULE_MESH_HANDLE);
				if (mesh)
				{
					cmd->BindAndDrawMesh(mesh, 1);
				}
			}

			if (entity.HasComponent<PlaneColliderComponent>())
			{

			}
		}
	}
}