#include "Graphics/SceneRenderer.h"

#include "Graphics/VertexLayout.h"
#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/CommandPool.h"

#include "Asset/Loader/GraphicsShaderLoader.h"
#include "ECS/Components.h"
#include "Timer.h"

#include "Engine Context/EngineAssets.h"

namespace Mule
{
	SceneRenderer::SceneRenderer(Ref<GraphicsContext> context, Ref<AssetManager> assetManager)
		:
		mGraphicsContext(context),
		mAssetManager(assetManager)
	{
		mGraph = RenderGraph::RenderGraph(context);
		mResourceUpdates.resize(mGraph.GetFrameCount());
		mTiming.resize(mGraph.GetFrameCount());

#pragma region Create Resources

		VertexLayout staticVertexLayout = VertexLayout()
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec2)
			.AddAttribute(AttributeType::Vec4);

		// Render passes
		{
			RenderPassDescription geometryPassDescription{};
			geometryPassDescription.Attachments = {
				{ TextureFormat::RGBA32F, true },
				{ TextureFormat::RG32UI, false },
				{ TextureFormat::R32F, false }
			};
			geometryPassDescription.DepthAttachment = { TextureFormat::D32F };
			geometryPassDescription.Subpasses = {
				{ { 0 }, true }
			};
			auto geometryRenderPass = mGraphicsContext->CreateRenderPass(geometryPassDescription);

			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				mGraph.AddResource(i, GEOMETRY_RENDER_PASS_ID, geometryRenderPass);
			}
		}

		// Framebuffers
		{
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
			framebufferDesc.RenderPass = mGraph.GetResource<RenderPass>(0, GEOMETRY_RENDER_PASS_ID);

			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				auto framebuffer = mGraphicsContext->CreateFrameBuffer(framebufferDesc);
				framebuffer->SetColorClearValue(0, glm::vec4(0, 0, 0, 1));
				framebuffer->SetColorClearValue(1, glm::ivec4(0));
				framebuffer->SetColorClearValue(2, glm::vec4(0));

				mGraph.AddResource(i, FRAMEBUFFER_ID, framebuffer);
			}
		}

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
			DescriptorSetLayoutDescription geometryShaderDSLD{};
			geometryShaderDSLD.Layouts = {
				LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Vertex | ShaderStage::Fragment), // Camera Buffer
				LayoutDescription(1, DescriptorType::UniformBuffer, ShaderStage::Fragment), // Material Buffer
				LayoutDescription(2, DescriptorType::UniformBuffer, ShaderStage::Fragment), // Light Buffer
				LayoutDescription(4, DescriptorType::Texture, ShaderStage::Fragment), // Light Buffer
				LayoutDescription(5, DescriptorType::Texture, ShaderStage::Fragment), // Light Buffer
				LayoutDescription(6, DescriptorType::Texture, ShaderStage::Fragment), // Light Buffer
			};
			auto geometryShaderDSL = mGraphicsContext->CreateDescriptorSetLayout(geometryShaderDSLD);

			DescriptorSetLayoutDescription bindlessTextureDSLD{};
			bindlessTextureDSLD.Layouts = {
				LayoutDescription(0, DescriptorType::Texture, ShaderStage::Fragment, 4096)
			};
			auto bindlessTextureDSL = mGraphicsContext->CreateDescriptorSetLayout(bindlessTextureDSLD);

			DescriptorSetLayoutDescription environmentShaderDSLD{};
			environmentShaderDSLD.Layouts = {
				LayoutDescription(0, DescriptorType::Texture, ShaderStage::Fragment),
				LayoutDescription(1, DescriptorType::UniformBuffer, ShaderStage::Vertex),
			};
			auto environmentShaderDSL = mGraphicsContext->CreateDescriptorSetLayout(environmentShaderDSLD);

			DescriptorSetLayoutDescription entityHighlightDSLD{};
			entityHighlightDSLD.Layouts = {
				LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Vertex)
			};
			auto entityHighlightDSL = mGraphicsContext->CreateDescriptorSetLayout(entityHighlightDSLD);

			DescriptorSetLayoutDescription entityOutlineDSLD{};
			entityOutlineDSLD.Layouts = {
				LayoutDescription(0, DescriptorType::StorageImage, ShaderStage::Compute),
				LayoutDescription(1, DescriptorType::Texture, ShaderStage::Compute),
			};
			auto entityOutlineDSL = mGraphicsContext->CreateDescriptorSetLayout(entityOutlineDSLD);

			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				mGraph.AddResource(i, GEOMETRY_SHADER_DSL_ID, geometryShaderDSL);
				mGraph.AddResource(i, BINDLESS_TEXTURE_DSL_ID, bindlessTextureDSL);
				mGraph.AddResource(i, ENVIRONMENT_SHADER_DSL_ID, environmentShaderDSL);
				mGraph.AddResource(i, ENTITY_HIGHLIGHT_DSL_ID, entityHighlightDSL);
				mGraph.AddResource(i, ENTITY_OUTLINE_DSL_ID, entityOutlineDSL);
			}
		}

		// Descriptor Sets
		{
			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				DescriptorSetDescription geometryShaderDSD{};
				geometryShaderDSD.Layouts = {
					mGraph.GetResource<DescriptorSetLayout>(i, GEOMETRY_SHADER_DSL_ID)
				};				
				auto geometryShaderDS = mGraphicsContext->CreateDescriptorSet(geometryShaderDSD);
				std::vector<DescriptorSetUpdate> geometryShaderDSUs = {
					DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) }),
					DescriptorSetUpdate(1, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, MATERIAL_BUFFER_ID) }),
					DescriptorSetUpdate(2, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, LIGHT_BUFFER_ID) })
				};
				geometryShaderDS->Update(geometryShaderDSUs);
				mGraph.AddResource(i, GEOMETRY_SHADER_DS_ID, geometryShaderDS);


				DescriptorSetDescription bindlessTextureDSD{};
				bindlessTextureDSD.Layouts = {
					mGraph.GetResource<DescriptorSetLayout>(i, BINDLESS_TEXTURE_DSL_ID)
				};
				auto bindlessTextureDS = mGraphicsContext->CreateDescriptorSet(bindlessTextureDSD);
				mGraph.AddResource(i, BINDLESS_TEXTURE_DS_ID, bindlessTextureDS);


				DescriptorSetDescription environmentShaderDSD{};
				environmentShaderDSD.Layouts = {
					mGraph.GetResource<DescriptorSetLayout>(i, ENVIRONMENT_SHADER_DSL_ID)
				};
				auto environmentShaderDS = mGraphicsContext->CreateDescriptorSet(environmentShaderDSD);
				std::vector<DescriptorSetUpdate> environmentShaderDSUs = {
					DescriptorSetUpdate(1, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) })
				};
				environmentShaderDS->Update(environmentShaderDSUs);
				mGraph.AddResource(i, ENVIRONMENT_SHADER_DS_ID, environmentShaderDS);

				DescriptorSetDescription entityHighlightDSD{};
				entityHighlightDSD.Layouts = {
					mGraph.GetResource<DescriptorSetLayout>(i, ENTITY_HIGHLIGHT_DSL_ID)
				};
				auto entityHighlightDS = mGraphicsContext->CreateDescriptorSet(entityHighlightDSD);
				std::vector<DescriptorSetUpdate> entityHighlightDSUs = {
					DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { mGraph.GetResource<UniformBuffer>(i, CAMERA_BUFFER_ID) })
				};
				entityHighlightDS->Update(entityHighlightDSUs);
				mGraph.AddResource(i, ENTITY_HIGHLIGHT_DS_ID, entityHighlightDS);


				DescriptorSetDescription entityOutlineDSD{};
				entityOutlineDSD.Layouts = {
					mGraph.GetResource<DescriptorSetLayout>(i, ENTITY_OUTLINE_DSL_ID)
				};
				auto entityOutlineDS = mGraphicsContext->CreateDescriptorSet(entityOutlineDSD);
				mGraph.AddResource(i, ENTITY_OUTLINE_DS_ID, entityOutlineDS);
			}
		}

		// Shaders
		{
			GraphicsShaderDescription opaqueGeometryDesc;
			opaqueGeometryDesc.SourcePath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
			opaqueGeometryDesc.Subpass = 0;
			opaqueGeometryDesc.RenderPass = mGraph.GetResource<RenderPass>(0, GEOMETRY_RENDER_PASS_ID);
			opaqueGeometryDesc.VertexLayout = staticVertexLayout;
			opaqueGeometryDesc.DescriptorLayouts = { mGraph.GetResource<DescriptorSetLayout>(0, GEOMETRY_SHADER_DSL_ID), mGraph.GetResource<DescriptorSetLayout>(0, BINDLESS_TEXTURE_DSL_ID) };
			opaqueGeometryDesc.PushConstants = {
				PushConstant(ShaderStage::Vertex, sizeof(glm::mat4)),
				PushConstant(ShaderStage::Fragment, 4 * sizeof(uint32_t)),
			};
			opaqueGeometryDesc.CulleMode = CullMode::Back;
			auto opaqueGeometryShader = mGraphicsContext->CreateGraphicsShader(opaqueGeometryDesc);
			mAssetManager->InsertAsset(opaqueGeometryShader);

			GraphicsShaderDescription transparentGeometryDesc;
			transparentGeometryDesc.SourcePath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
			transparentGeometryDesc.Subpass = 0;
			transparentGeometryDesc.RenderPass = mGraph.GetResource<RenderPass>(0, GEOMETRY_RENDER_PASS_ID);
			transparentGeometryDesc.VertexLayout = staticVertexLayout;
			transparentGeometryDesc.DescriptorLayouts = { mGraph.GetResource<DescriptorSetLayout>(0, GEOMETRY_SHADER_DSL_ID), mGraph.GetResource<DescriptorSetLayout>(0, BINDLESS_TEXTURE_DSL_ID) };
			transparentGeometryDesc.PushConstants = {
				PushConstant(ShaderStage::Vertex, sizeof(glm::mat4)),
				PushConstant(ShaderStage::Fragment, 4 * sizeof(uint32_t)),
			};
			transparentGeometryDesc.CulleMode = CullMode::None;
			transparentGeometryDesc.Macros = { { "TRANSPARENCY", "" }};
			transparentGeometryDesc.EnableDepthTesting = true;
			transparentGeometryDesc.BlendEnable = true;
			transparentGeometryDesc.WriteDepth = false;
			auto transparentGeometryShader = mGraphicsContext->CreateGraphicsShader(transparentGeometryDesc);
			mAssetManager->InsertAsset(transparentGeometryShader);

			GraphicsShaderDescription environemntShaderDesc{};
			environemntShaderDesc.SourcePath = "../Assets/Shaders/Graphics/EnvironmentMapShader.glsl";
			environemntShaderDesc.Subpass = 0;
			environemntShaderDesc.RenderPass = mGraph.GetResource<RenderPass>(0, GEOMETRY_RENDER_PASS_ID);
			environemntShaderDesc.VertexLayout = staticVertexLayout;
			environemntShaderDesc.DescriptorLayouts = { mGraph.GetResource<DescriptorSetLayout>(0, ENVIRONMENT_SHADER_DSL_ID)};
			environemntShaderDesc.PushConstants = { };
			environemntShaderDesc.CulleMode = CullMode::Front;
			environemntShaderDesc.WriteDepth = false;
			environemntShaderDesc.EnableDepthTesting = false;
			auto environmentShader = mGraphicsContext->CreateGraphicsShader(environemntShaderDesc);

			GraphicsShaderDescription entityHighlightShaderDesc{};
			entityHighlightShaderDesc.SourcePath = "../Assets/Shaders/Graphics/HighlightEntityShader.glsl";
			entityHighlightShaderDesc.Subpass = 0;
			entityHighlightShaderDesc.RenderPass = mGraph.GetResource<RenderPass>(0, GEOMETRY_RENDER_PASS_ID);
			entityHighlightShaderDesc.VertexLayout = staticVertexLayout;
			entityHighlightShaderDesc.DescriptorLayouts = { mGraph.GetResource<DescriptorSetLayout>(0, ENTITY_HIGHLIGHT_DSL_ID) };
			entityHighlightShaderDesc.PushConstants = {
				PushConstant(ShaderStage::Vertex, sizeof(glm::mat4))
			};
			entityHighlightShaderDesc.CulleMode = CullMode::Front;
			entityHighlightShaderDesc.WriteDepth = false;
			entityHighlightShaderDesc.EnableDepthTesting = false;
			auto entityHighlightShader = mGraphicsContext->CreateGraphicsShader(entityHighlightShaderDesc);
			mAssetManager->InsertAsset(entityHighlightShader);

			ComputeShaderDescription entityOutlineShaderDesc{};
			entityOutlineShaderDesc.Filepath = "../Assets/Shaders/Compute/HighlightShader.glsl";
			entityOutlineShaderDesc.Layouts = { mGraph.GetResource<DescriptorSetLayout>(0, ENTITY_OUTLINE_DSL_ID) };
			auto entityOutlineShader = mGraphicsContext->CreateComputeShader(entityOutlineShaderDesc);

			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				mGraph.AddResource(i, OPAQUE_GEOMETRY_SHADER_ID, opaqueGeometryShader);
				mGraph.AddResource(i, TRANPARENT_GEOMETRY_SHADER_ID, transparentGeometryShader);
				mGraph.AddResource(i, ENVIRONMENT_SHADER_ID, environmentShader);
				mGraph.AddResource(i, ENTITY_HIGHLIGHT_SHADER_ID, entityHighlightShader);
				mGraph.AddResource(i, ENTITY_OUTLINE_SHADER_ID, entityOutlineShader);
			}
		}

#pragma endregion

#pragma region Render Graph Init

		mGraph.AddPass(
			ENVIRONMENT_PASS_NAME,
			{ },
			std::bind(&SceneRenderer::RenderEnvironmentCallback, this, std::placeholders::_1));

		mGraph.AddPass(
			GEOMETRY_PASS_NAME,
			{ ENVIRONMENT_PASS_NAME },
			std::bind(&SceneRenderer::RenderSolidGeometryCallback, this, std::placeholders::_1));

		mGraph.AddPass(
			TRANPARENT_GEOMETRY_PASS_NAME,
			{ GEOMETRY_PASS_NAME },
			std::bind(&SceneRenderer::RenderTransparentGeometryCallback, this, std::placeholders::_1));

		mGraph.AddPass(
			ENTITY_HIGHLIGHT_PASS_NAME,
			{ TRANPARENT_GEOMETRY_PASS_NAME },
			std::bind(&SceneRenderer::RenderEntityHighlightCallback, this, std::placeholders::_1));

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
			if (material->AlbedoMap != NullAssetHandle)
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

	void SceneRenderer::OnEditorRender(WeakRef<Scene> scene, const Camera& camera, const std::vector<WeakRef<Semaphore>>& waitSemaphores)
	{
		auto& timingInfo = mTiming[mGraph.GetFrameIndex()];
		// We call next frame first so all graph queries after render will reflect the currect frame
		mGraph.NextFrame();
		mGraph.Wait();
		
		Timer dataPrepTimer;
		dataPrepTimer.Start();
		// Data Prep
		{
			WeakRef<UniformBuffer> cameraUB = mGraph.QueryResource<UniformBuffer>(CAMERA_BUFFER_ID);
			WeakRef<UniformBuffer> lightUB = mGraph.QueryResource<UniformBuffer>(LIGHT_BUFFER_ID);
			WeakRef<UniformBuffer> materialBuffer = mGraph.QueryResource<DescriptorSet>(MATERIAL_BUFFER_ID);
			WeakRef<DescriptorSet> bindlessTextureDS = mGraph.QueryResource<DescriptorSet>(BINDLESS_TEXTURE_DS_ID);
			WeakRef<FrameBuffer> framebuffer = mGraph.QueryResource<FrameBuffer>(FRAMEBUFFER_ID);

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
				framebuffer->Resize(resourceUpdate.ResizeWidth, resourceUpdate.ResizeHeight);

				auto queue = mGraphicsContext->GetGraphicsQueue();
				auto cmdPool = queue->CreateCommandPool();
				auto cmd = cmdPool->CreateCommandBuffer();
				auto fence = mGraphicsContext->CreateFence();

				for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
				{
					fence->Reset();

					cmd->Reset();
					cmd->Begin();
					cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::General);
					cmd->TranistionImageLayout(framebuffer->GetColorAttachment(2), ImageLayout::ShaderReadOnly);
					cmd->End();
					queue->Submit({ cmd }, {}, {}, fence);
					fence->Wait();

					auto outlineDS = mGraph.QueryResource<DescriptorSet>(ENTITY_OUTLINE_DS_ID);

					std::vector<DescriptorSetUpdate> outlineDSUs = {
						DescriptorSetUpdate(0, DescriptorType::StorageImage, 0, { framebuffer->GetColorAttachment(0) }, {}),
						DescriptorSetUpdate(1, DescriptorType::Texture, 0, { framebuffer->GetColorAttachment(2) }, {}),
					};

					outlineDS->Update(outlineDSUs);

					fence->Reset();

					cmd->Reset();
					cmd->Begin();
					cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
					cmd->TranistionImageLayout(framebuffer->GetColorAttachment(2), ImageLayout::ColorAttachment);
					cmd->End();
					queue->Submit({ cmd }, {}, {}, fence);
					fence->Wait();					
				}
			}
		}
		dataPrepTimer.Stop();
		timingInfo.CPUPrepareTime = dataPrepTimer.Query();

		mGraph.SetCamera(camera);
		mGraph.SetScene(scene);

		Timer executionTimer;
		executionTimer.Start();
		
		mGraph.Execute(waitSemaphores);
		
		executionTimer.Stop();
		timingInfo.CPUExecutionTime = executionTimer.Query();
		timingInfo.RenderPassStats = mGraph.GetRenderPassStats();
	}

	void SceneRenderer::OnRender(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores)
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

		if (camera)
		{
			OnEditorRender(scene, *camera, waitSemaphores);
		}
	}

	Guid SceneRenderer::Pick(uint32_t x, uint32_t y)
	{
		mGraph.Wait();
		uint32_t index = mGraph.GetFrameIndex();
		auto framebuffer = mGraph.GetResource<FrameBuffer>(index, FRAMEBUFFER_ID);

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
		return mGraph.QueryResource<FrameBuffer>(FRAMEBUFFER_ID);
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
			if (handle != NullAssetHandle)
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

	void SceneRenderer::RenderSolidGeometryCallback(const RenderGraph::PassContext& ctx)
	{
		WeakRef<FrameBuffer> framebuffer = ctx.Get<FrameBuffer>(FRAMEBUFFER_ID);
		WeakRef<RenderPass> renderPass = ctx.Get<RenderPass>(GEOMETRY_RENDER_PASS_ID);
		WeakRef<CommandBuffer> cmd = ctx.GetCommandBuffer();
		WeakRef<GraphicsShader> shader = ctx.Get<GraphicsShader>(OPAQUE_GEOMETRY_SHADER_ID);
		WeakRef<DescriptorSet> geometryDS = ctx.Get<FrameBuffer>(GEOMETRY_SHADER_DS_ID);
		WeakRef<DescriptorSet> bindlessTextureDS = ctx.Get<FrameBuffer>(BINDLESS_TEXTURE_DS_ID);
		WeakRef<Scene> scene = ctx.GetScene();

		if (!shader->IsValid())
			return;

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
				DescriptorSetUpdate(4, DescriptorType::Texture, 0, { irradianceMap }, {}),
				DescriptorSetUpdate(5, DescriptorType::Texture, 0, { prefilterMap }, {}),
				DescriptorSetUpdate(6, DescriptorType::Texture, 0, { brdfLutMap }, {}),
			};
			geometryDS->Update(geometryShaderDSUs);
		}
		else
		{
			auto blackCubeMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);
			auto blackTexture = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_HANDLE);
			std::vector<DescriptorSetUpdate> geometryShaderDSUs = {
				DescriptorSetUpdate(4, DescriptorType::Texture, 0, { blackCubeMap }, {}),
				DescriptorSetUpdate(5, DescriptorType::Texture, 0, { blackCubeMap }, {}),
				DescriptorSetUpdate(6, DescriptorType::Texture, 0, { blackTexture }, {}),
			};
			geometryDS->Update(geometryShaderDSUs);
		}
				
		cmd->BeginRenderPass(framebuffer, renderPass, false);

		cmd->BindGraphicsPipeline(shader);
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

		cmd->EndRenderPass();
	}

	void SceneRenderer::RenderTransparentGeometryCallback(const RenderGraph::PassContext& ctx)
	{
		WeakRef<FrameBuffer> framebuffer = ctx.Get<FrameBuffer>(FRAMEBUFFER_ID);
		WeakRef<RenderPass> renderPass = ctx.Get<RenderPass>(GEOMETRY_RENDER_PASS_ID);
		WeakRef<CommandBuffer> cmd = ctx.GetCommandBuffer();
		WeakRef<GraphicsShader> shader = ctx.Get<GraphicsShader>(TRANPARENT_GEOMETRY_SHADER_ID);
		WeakRef<DescriptorSet> geometryDS = ctx.Get<FrameBuffer>(GEOMETRY_SHADER_DS_ID);
		WeakRef<DescriptorSet> bindlessTextureDS = ctx.Get<FrameBuffer>(BINDLESS_TEXTURE_DS_ID);
		WeakRef<Scene> scene = ctx.GetScene();

		if (!shader->IsValid())
			return;

		cmd->BeginRenderPass(framebuffer, renderPass, false);

		cmd->BindGraphicsPipeline(shader);
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

		cmd->EndRenderPass();
	}

	void SceneRenderer::RenderEnvironmentCallback(const RenderGraph::PassContext& ctx)
	{
		WeakRef<CommandBuffer> cmd = ctx.GetCommandBuffer();
		WeakRef<FrameBuffer> framebuffer = ctx.Get<FrameBuffer>(FRAMEBUFFER_ID);
		WeakRef<RenderPass> renderPass = ctx.Get<FrameBuffer>(GEOMETRY_RENDER_PASS_ID);
		WeakRef<GraphicsShader> shader = ctx.Get<GraphicsShader>(ENVIRONMENT_SHADER_ID);
		WeakRef<DescriptorSet> DS = ctx.Get<DescriptorSet>(ENVIRONMENT_SHADER_DS_ID);
		WeakRef<Scene> scene = ctx.GetScene();

		if (!shader->IsValid())
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

		std::vector<DescriptorSetUpdate> DSUs = {
			DescriptorSetUpdate(0, DescriptorType::Texture, 0, { cubeMap }, {})
		};
		DS->Update(DSUs);

		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment); // TODO: to be handled by scene graph
		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(1), ImageLayout::ColorAttachment); // TODO: to be handled by scene graph
		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(2), ImageLayout::ColorAttachment); // TODO: to be handled by scene graph

		cmd->BeginRenderPass(framebuffer, renderPass, true);
		cmd->BindGraphicsPipeline(shader);
		cmd->BindGraphicsDescriptorSet(shader, { DS });
		auto mesh = mAssetManager->GetAsset<Mesh>(MULE_CUBE_MESH_HANDLE);
		if (mesh)
		{
			cmd->BindAndDrawMesh(mesh, 1);
		}
		cmd->EndRenderPass();
	}

	bool SceneRenderer::RenderEntityChildrenHighlight(Entity e, WeakRef<CommandBuffer> cmd, WeakRef<GraphicsShader> shader)
	{
		bool ret = false;
		if (e.HasComponent<MeshComponent>())
		{
			glm::mat4 transform = e.GetTransform();
			auto& meshComponent = e.GetComponent<MeshComponent>();

			auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);

			if (mesh)
			{
				cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				cmd->BindAndDrawMesh(mesh, 1);
				ret = true;
			}
		}

		for (auto child : e.Children())
		{
			ret |= RenderEntityChildrenHighlight(child, cmd, shader);
		}

		return ret;
	}

	void SceneRenderer::RenderEntityHighlightCallback(const RenderGraph::PassContext& ctx)
	{
		WeakRef<CommandBuffer> cmd = ctx.GetCommandBuffer();
		WeakRef<Scene> scene = ctx.GetScene();
		WeakRef<GraphicsShader> shader = ctx.Get<GraphicsShader>(ENTITY_HIGHLIGHT_SHADER_ID);
		WeakRef<ComputeShader> outlineShader = ctx.Get<ComputeShader>(ENTITY_OUTLINE_SHADER_ID);
		WeakRef<FrameBuffer> framebuffer = ctx.Get<FrameBuffer>(FRAMEBUFFER_ID);
		WeakRef<RenderPass> renderpass = ctx.Get<RenderPass>(GEOMETRY_RENDER_PASS_ID);
		WeakRef<DescriptorSet> DS = ctx.Get<DescriptorSet>(ENTITY_HIGHLIGHT_DS_ID);
		WeakRef<DescriptorSet> outlineDS = ctx.Get<DescriptorSet>(ENTITY_OUTLINE_DS_ID);

		cmd->BeginRenderPass(framebuffer, renderpass);

		cmd->BindGraphicsPipeline(shader);
		cmd->BindGraphicsDescriptorSet(shader, { DS });

		bool hasHighlight = false;
		for (auto entity : scene->Iterate<HighlightComponent>())
		{
			hasHighlight |= RenderEntityChildrenHighlight(entity, cmd, shader);			
		}

		cmd->EndRenderPass();

		// Dont run expensive compute shader if no highlight
		if (!hasHighlight)
		{
			cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly); // TODO: to be handled by scene graph
			return;
		}

		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::General);
		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(1), ImageLayout::ShaderReadOnly);
		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(2), ImageLayout::ShaderReadOnly);

		cmd->BindComputePipeline(outlineShader);
		cmd->BindComputeDescriptorSet(outlineShader, outlineDS);

		uint32_t width = framebuffer->GetWidth();
		uint32_t height = framebuffer->GetHeight();
		cmd->Execute(width / 16, height / 16, 1);

		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly); // TODO: to be handled by scene graph
	}
}