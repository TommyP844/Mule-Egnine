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

		// Descriptor Set Layouts
		Ref<DescriptorSetLayout> geometryShaderDSL;
		Ref<DescriptorSetLayout> bindlessTextureDSL;

#pragma region Create Resources

		VertexLayout staticVertexLayout = VertexLayout()
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec2)
			.AddAttribute(AttributeType::Vec4);

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
			geometryShaderDSL = mGraphicsContext->CreateDescriptorSetLayout(geometryShaderDSLD);

			DescriptorSetLayoutDescription bindlessTextureDSLD{};
			bindlessTextureDSLD.Layouts = {
				LayoutDescription(0, DescriptorType::Texture, ShaderStage::Fragment, 4096)
			};
			bindlessTextureDSL = mGraphicsContext->CreateDescriptorSetLayout(bindlessTextureDSLD);

			DescriptorSetLayoutDescription environmentShaderDSLD{};
			environmentShaderDSLD.Layouts = {
				LayoutDescription(0, DescriptorType::Texture, ShaderStage::Fragment),
				LayoutDescription(1, DescriptorType::UniformBuffer, ShaderStage::Vertex),
			};
			auto environmentShaderDSL = mGraphicsContext->CreateDescriptorSetLayout(environmentShaderDSLD);

			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				mGraph.AddResource(i, GEOMETRY_SHADER_DSL_ID, geometryShaderDSL);
				mGraph.AddResource(i, BINDLESS_TEXTURE_DSL_ID, bindlessTextureDSL);
				mGraph.AddResource(i, ENVIRONMENT_SHADER_DSL_ID, environmentShaderDSL);
			}
		}

		// Descriptor Sets
		{
			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				DescriptorSetDescription geometryShaderDSD{};
				geometryShaderDSD.Layouts = {
					geometryShaderDSL
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
					bindlessTextureDSL
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
			}
		}

		// Render passes
		{
			RenderPassDescription geometryPassDescription{};
			geometryPassDescription.Attachments = {
				{ TextureFormat::RGBA32F, true },
				{ TextureFormat::R32UI, false },
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
				{ TextureFormat::RGBA32F, TextureFlags::None },
				{ TextureFormat::R32UI, TextureFlags::None },
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

		// Shaders
		{
			GraphicsShaderDescription opaqueGeometryDesc;
			opaqueGeometryDesc.SourcePath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
			opaqueGeometryDesc.Subpass = 0;
			opaqueGeometryDesc.RenderPass = mGraph.GetResource<RenderPass>(0, GEOMETRY_RENDER_PASS_ID);
			opaqueGeometryDesc.VertexLayout = staticVertexLayout;
			opaqueGeometryDesc.DescriptorLayouts = { geometryShaderDSL, bindlessTextureDSL };
			opaqueGeometryDesc.PushConstants = {
				PushConstant(ShaderStage::Vertex, sizeof(glm::mat4)),
				PushConstant(ShaderStage::Fragment, 2 * sizeof(uint32_t)),
			};
			opaqueGeometryDesc.CulleMode = CullMode::Back;
			auto opaqueGeometryShader = mGraphicsContext->CreateGraphicsShader(opaqueGeometryDesc);
			mAssetManager->InsertAsset(opaqueGeometryShader);

			GraphicsShaderDescription transparentGeometryDesc;
			transparentGeometryDesc.SourcePath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
			transparentGeometryDesc.Subpass = 0;
			transparentGeometryDesc.RenderPass = mGraph.GetResource<RenderPass>(0, GEOMETRY_RENDER_PASS_ID);
			transparentGeometryDesc.VertexLayout = staticVertexLayout;
			transparentGeometryDesc.DescriptorLayouts = { geometryShaderDSL, bindlessTextureDSL };
			transparentGeometryDesc.PushConstants = {
				PushConstant(ShaderStage::Vertex, sizeof(glm::mat4)),
				PushConstant(ShaderStage::Fragment, 2 * sizeof(uint32_t)),
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

			for (uint32_t i = 0; i < mGraph.GetFrameCount(); i++)
			{
				mGraph.AddResource(i, OPAQUE_GEOMETRY_SHADER_ID, opaqueGeometryShader);
				mGraph.AddResource(i, TRANPARENT_GEOMETRY_SHADER_ID, transparentGeometryShader);
				mGraph.AddResource(i, ENVIRONMENT_SHADER_ID, environmentShader);
			}
		}

#pragma endregion

#pragma region Render Graph Init

		mGraph.AddPass(ENVIRONMENT_PASS_NAME,
			{ },
			std::bind(&SceneRenderer::RenderEnvironmentCallback, this, std::placeholders::_1));

		mGraph.AddPass(GEOMETRY_PASS_NAME,
			{ ENVIRONMENT_PASS_NAME },
			std::bind(&SceneRenderer::RenderSolidGeometryCallback, this, std::placeholders::_1));

		mGraph.AddPass(TRANPARENT_GEOMETRY_PASS_NAME,
			{ GEOMETRY_PASS_NAME },
			std::bind(&SceneRenderer::RenderTransparentGeometryCallback, this, std::placeholders::_1));

		mGraph.Compile();
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
		for (int i = 0; i < 2; i++)
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

		gpuMaterial.AlbedoIndex = mTextureArray.QueryIndex(material->AlbedoMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AlbedoMap);
		gpuMaterial.NormalIndex = mTextureArray.QueryIndex(material->NormalMap) == UINT32_MAX ? -1 : mTextureArray.QueryIndex(material->NormalMap);
		gpuMaterial.MetalnessIndex = mTextureArray.QueryIndex(material->MetalnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->MetalnessMap);
		gpuMaterial.RoughnessIndex = mTextureArray.QueryIndex(material->RoughnessMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->RoughnessMap);
		gpuMaterial.AOIndex = mTextureArray.QueryIndex(material->AOMap) == UINT32_MAX ? mWhiteImageIndex : mTextureArray.QueryIndex(material->AOMap);
		gpuMaterial.EmissiveIndex = mTextureArray.QueryIndex(material->EmissiveMap) == UINT32_MAX ? mBlackImageIndex : mTextureArray.QueryIndex(material->EmissiveMap);
		gpuMaterial.OpacityIndex = mTextureArray.QueryIndex(material->OpacityMap) == UINT32_MAX ? -1 : mTextureArray.QueryIndex(material->OpacityMap);

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

				for (auto entityId : scene->Iterate<DirectionalLightComponent>())
				{
					Entity e((uint32_t)entityId, scene);
					DirectionalLightComponent& directionalLight = e.GetComponent<DirectionalLightComponent>();
					if (!directionalLight.Active)
						continue;

					lightData.DirectionalLight.Color = directionalLight.Color;
					lightData.DirectionalLight.Intensity = directionalLight.Intensity;
					glm::quat rotation = glm::quat(glm::radians(e.GetTransformComponent().Rotation));
					glm::vec4 direction = rotation * glm::vec4(0, -1, 0, 0);
					lightData.DirectionalLight.Direction = glm::normalize(direction);

					break;
				}

				for (auto entityId : scene->Iterate<PointLightComponent>())
				{
					Entity e((uint32_t)entityId, scene);

					PointLightComponent& pointLight = e.GetComponent<PointLightComponent>();

					if (!pointLight.Active)
						continue;

					lightData.PointLights[lightData.NumPointLights].Color = pointLight.Color;
					lightData.PointLights[lightData.NumPointLights].Intensity = pointLight.Radiance;
					lightData.PointLights[lightData.NumPointLights++].Position = e.GetTransformComponent().Translation;
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
		Camera camera;
		for (auto entityId : scene->Iterate<CameraComponent>())
		{
			Entity e((uint32_t)entityId, scene);
			CameraComponent& cameraComponent = e.GetComponent<CameraComponent>();
			if (!cameraComponent.Active)
				continue;
			camera = cameraComponent.Camera;
			break;
		}		

		OnEditorRender(scene, camera, waitSemaphores);
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
		for (auto entityId : scene->Iterate<EnvironmentMapComponent>())
		{
			Entity e((uint32_t)entityId, scene);
			const auto& envMapComponent = e.GetComponent<EnvironmentMapComponent>();
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

		for (auto entityId : scene->Iterate<MeshComponent>())
		{
			Entity e((uint32_t)entityId, scene);

			const MeshComponent& meshComponent = e.GetComponent<MeshComponent>();
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

			glm::mat4 transform = e.GetTransformComponent().TRS();
			Entity parent = e.Parent();
			while (parent)
			{
				transform *= parent.GetTransformComponent().TRS();
				Entity p = e.Parent();
				if (p.ID() == parent.ID())
					break;
				parent = e.Parent();
			}

			// set push constants
			uint32_t fragmentConstants[] = {
				materialIndex,
				0 // unused
			};

			cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
			cmd->SetPushConstants(shader, ShaderStage::Fragment, &fragmentConstants[0], sizeof(uint32_t) * 2);

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

		for (auto entityId : scene->Iterate<MeshComponent>())
		{
			Entity e((uint32_t)entityId, scene);

			const MeshComponent& meshComponent = e.GetComponent<MeshComponent>();
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

			glm::mat4 transform = e.GetTransformComponent().TRS();
			Entity parent = e.Parent();
			while (parent)
			{
				transform *= parent.GetTransformComponent().TRS();
				Entity p = e.Parent();
				if (p.ID() == parent.ID())
					break;
				parent = e.Parent();
			}

			// set push constants
			uint32_t fragmentConstants[] = {
				materialIndex,
				0 // unused
			};

			cmd->SetPushConstants(shader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
			cmd->SetPushConstants(shader, ShaderStage::Fragment, &fragmentConstants[0], sizeof(uint32_t) * 2);

			cmd->BindAndDrawMesh(mesh, 1);
		}

		cmd->EndRenderPass();

		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly); // TODO: to be handles by scene graph
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
		for (auto entityId : scene->Iterate<EnvironmentMapComponent>())
		{
			Entity e((uint32_t)entityId, scene);
			const auto& environmentMapComponent = e.GetComponent<EnvironmentMapComponent>();
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

		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment); // TODO: to be handles by scene graph
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

	void SceneRenderer::RenderEntityHighlightCallback(const RenderGraph::PassContext& ctx)
	{
		WeakRef<CommandBuffer> cmd = ctx.GetCommandBuffer();
		WeakRef<GraphicsShader> shader = ctx.Get<GraphicsShader>("EntityHighlightShader");
		WeakRef<FrameBuffer> framebuffer = ctx.Get<FrameBuffer>(FRAMEBUFFER_ID);
		WeakRef<DescriptorSet> DS = ctx.Get<FrameBuffer>("HighlightDS");

		cmd->BindComputePipeline(shader);
		uint32_t width = framebuffer->GetWidth();
		uint32_t height = framebuffer->GetHeight();
		uint32_t sizes[] = {
			width,
			height
		};
		cmd->BindComputeDescriptorSet(shader, DS);
		cmd->SetPushConstants(shader, &sizes[0], sizeof(uint32_t) * 2);
		cmd->Execute(width / 16, height / 16, 1);
	}
}