
#include "Graphics/Renderer/Renderer.h"

#include "Graphics/GPUObjects.h"
#include "Graphics/ShaderFactory.h"

#include "Graphics/API/Texture2DArray.h" 

#include "ScopedBuffer.h"

namespace Mule
{
	Renderer* Renderer::sRenderer = nullptr;

	Renderer::Renderer()
		:
		mFramesInFlight(2),
		mFrameIndex(0)
	{
		mResourceUpdates.resize(mFramesInFlight);
	}

	void Renderer::Init()
	{
		assert(!sRenderer && "Renderer has already been initialized");
		sRenderer = new Renderer();

		uint8_t whiteImageData[] = {
			255, 255, 255, 255,		255, 255, 255, 255,
			255, 255, 255, 255,		255, 255, 255, 255
		};

		Buffer whiteImageBuffer(whiteImageData, sizeof(whiteImageData));

		sRenderer->mWhiteTex = Texture2D::Create("White Texture", whiteImageBuffer, 2, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);

		uint8_t blackImageData[] = {
			0, 0, 0, 255,	0, 0, 0, 255,
			0, 0, 0, 255,	0, 0, 0, 255
		};

		Buffer blackImageBuffer(blackImageData, sizeof(blackImageData));

		sRenderer->mBlackTex = Texture2D::Create("Black Texture", blackImageBuffer, 2, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);

		uint8_t normalImageData[] = {
			128, 128, 255, 255,		128, 128, 255, 255,
			128, 128, 255, 255,		128, 128, 255, 255
		};

		Buffer normalImageBuffer(normalImageData, sizeof(normalImageData));

		sRenderer->mDefaultNormalTex = Texture2D::Create("Normal Texture", normalImageBuffer, 2, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);

		sRenderer->mDefaultMaterial = MakeRef<Material>();
		sRenderer->mDefaultMaterial->AlbedoMap = sRenderer->mWhiteTex->Handle();
		sRenderer->mDefaultMaterial->NormalMap = sRenderer->mDefaultNormalTex->Handle();
		sRenderer->mDefaultMaterial->MetalnessMap = sRenderer->mWhiteTex->Handle();
		sRenderer->mDefaultMaterial->RoughnessMap = sRenderer->mWhiteTex->Handle();
		sRenderer->mDefaultMaterial->AOMap = sRenderer->mWhiteTex->Handle();
		sRenderer->mDefaultMaterial->OpacityMap = sRenderer->mWhiteTex->Handle();
		sRenderer->mDefaultMaterial->EmissiveMap = sRenderer->mBlackTex->Handle();

		sRenderer->AddTexture(sRenderer->mWhiteTex);
		sRenderer->AddTexture(sRenderer->mBlackTex);
		sRenderer->AddTexture(sRenderer->mDefaultNormalTex);

		sRenderer->AddMaterial(sRenderer->mDefaultMaterial);

		sRenderer->mBindlessMaterialSRGHandle = ResourceHandle("Bindless.Material.SRG", ResourceType::ShaderResourceGroup);
		sRenderer->mBindlessTextureSRGHandle = ResourceHandle("Bindless.Texture.SRG", ResourceType::ShaderResourceGroup);
		sRenderer->mBindlessMaterialBufferHandle = ResourceHandle("Bindless.Material.Buffer", ResourceType::UniformBuffer);

		for (uint32_t i = 0; i < sRenderer->mFramesInFlight; i++)
		{
			sRenderer->mBindlessMaterialBuffer.push_back(UniformBuffer::Create(sizeof(GPU::Material) * 800));
			sRenderer->mBindlessMaterialSRG.push_back(ShaderResourceGroup::Create({
				ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Fragment)
				}));

			sRenderer->mBindlessTextureSRG.push_back(ShaderResourceGroup::Create({
				ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Fragment, 4096)
				}));

			auto bindlessMaterialSRG = sRenderer->mBindlessMaterialSRG[i];
			auto bindlessMaterialBuffer = sRenderer->mBindlessMaterialBuffer[i];

			bindlessMaterialSRG->Update(0, bindlessMaterialBuffer);
		}

		ShaderFactory::Init();
		ShaderFactory& shaderFactory = ShaderFactory::Get();

		// Graphics Pipelines
		{
			VertexLayout defaultVertexLayout;
			defaultVertexLayout.AddAttribute(AttributeType::Vec3)
				.AddAttribute(AttributeType::Vec3)
				.AddAttribute(AttributeType::Vec3)
				.AddAttribute(AttributeType::Vec2)
				.AddAttribute(AttributeType::Vec4);


			GraphicsPipelineDescription geometryPipeline{};
			geometryPipeline.Filepath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
			geometryPipeline.FilleMode = FillMode::Solid;
			geometryPipeline.CullMode = CullMode::Back;
			geometryPipeline.VertexLayout = defaultVertexLayout;
			geometryPipeline.DepthFormat = TextureFormat::D_32F;
			geometryPipeline.EnableDepthTest = true;
			geometryPipeline.EnableDepthWrite = true;
			shaderFactory.RegisterGraphicsPipeline("Geometry", geometryPipeline);


			GraphicsPipelineDescription environmentMapPipeline{};
			environmentMapPipeline.Filepath = "../Assets/Shaders/Graphics/EnvironmentMapShader.glsl";
			environmentMapPipeline.FilleMode = FillMode::Solid;
			environmentMapPipeline.CullMode = CullMode::Front;
			environmentMapPipeline.VertexLayout = defaultVertexLayout;
			environmentMapPipeline.DepthFormat = TextureFormat::D_32F;
			environmentMapPipeline.EnableDepthTest = true;
			environmentMapPipeline.EnableDepthWrite = false;
			environmentMapPipeline.DepthFunc = DepthFunc::LessEqual;
			shaderFactory.RegisterGraphicsPipeline("EnvironmentMap", environmentMapPipeline);

			GraphicsPipelineDescription shadowDepthPipeline{};
			shadowDepthPipeline.Filepath = "../Assets/Shaders/Graphics/ShadowDepthShader.glsl";
			shadowDepthPipeline.FilleMode = FillMode::Solid;
			shadowDepthPipeline.CullMode = CullMode::Front;
			shadowDepthPipeline.VertexLayout = defaultVertexLayout;
			shadowDepthPipeline.DepthFormat = TextureFormat::D_32F;
			shadowDepthPipeline.EnableDepthTest = true;
			shadowDepthPipeline.EnableDepthWrite = true;
			shadowDepthPipeline.DepthFunc = DepthFunc::LessEqual;
			shaderFactory.RegisterGraphicsPipeline("ShadowDepth", shadowDepthPipeline);
		}

		// Compute Pipelines
		{
			ComputePipelineDescription cubeMapCompute{};
			cubeMapCompute.Filepath = "../Assets/Shaders/Compute/HDRToCubeMapCompute.glsl";
			shaderFactory.RegisterComputePipeline("HDRToCubemap", cubeMapCompute);

			ComputePipelineDescription diffuseIBLCompute{};
			diffuseIBLCompute.Filepath = "../Assets/Shaders/Compute/DiffuseIBLCompute.glsl";
			shaderFactory.RegisterComputePipeline("DiffuseIBL", diffuseIBLCompute);

			ComputePipelineDescription prefilterIBLCompute{};
			prefilterIBLCompute.Filepath = "../Assets/Shaders/Compute/PrefilterIBLCompute.glsl";
			shaderFactory.RegisterComputePipeline("PrefilterIBL", prefilterIBLCompute);

			ComputePipelineDescription deferredLightingCompute{};
			deferredLightingCompute.Filepath = "../Assets/Shaders/Compute/DeferredLightingPass.glsl";
			shaderFactory.RegisterComputePipeline("DeferredLighting", deferredLightingCompute);
		}


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
		Ref<ResourceRegistry> registry = MakeRef<ResourceRegistry>(mFramesInFlight, mResourceBuilder);

		mRenderGraph->InitializeRegistry(*registry);

		registry->InsertResources(mBindlessMaterialSRGHandle, mBindlessMaterialSRG);
		registry->InsertResources(mBindlessTextureSRGHandle, mBindlessTextureSRG);

		registry->SetOutputHandle(ResourceHandle("MainOutput", ResourceType::RenderTarget));

		return registry;
	}

	void Renderer::Submit(const Camera& camera, const CommandList& commandList)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mRenderRequests.push_back({
			.Camera = camera,
			.Commands = commandList,
		});		
	}

	void Renderer::Render()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (BindlessResourcesNeedUpdate())
		{
			for (auto renderRequest : mRenderRequests)
			{
				auto registry = renderRequest.Camera.GetRegistry();
				if (!registry)
					continue;
				registry->WaitForFences(mFrameIndex);
			}

			UpdateBindlessResources();
		}

		for (auto renderRequest : mRenderRequests)
		{
			mRenderGraph->Execute(renderRequest.Commands, renderRequest.Camera, mFrameIndex);
		}

		mRenderRequests.clear();

		mFrameIndex ^= 1;
	}

	void Renderer::AddTexture(WeakRef<Texture> texture)
	{
		std::lock_guard<std::mutex> lock(mResourceMutex);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mResourceUpdates[i].AddTextures.push_back(texture);
		}
	}

	void Renderer::RemoveTexture(WeakRef<Texture> texture)
	{
		std::lock_guard<std::mutex> lock(mResourceMutex);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mResourceUpdates[i].RemoveTextures.push_back(texture);
		}
	}

	void Renderer::AddMaterial(WeakRef<Material> material)
	{
		std::lock_guard<std::mutex> lock(mResourceMutex);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mResourceUpdates[i].AddMaterials.push_back(material);
		}
	}

	void Renderer::UpdateMaterial(WeakRef<Material> material)
	{
		std::lock_guard<std::mutex> lock(mResourceMutex);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mResourceUpdates[i].UpdateMaterials.push_back(material);
		}
	}

	void Renderer::RemoveMaterial(WeakRef<Material> material)
	{
		std::lock_guard<std::mutex> lock(mResourceMutex);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mResourceUpdates[i].RemoveMaterials.push_back(material);
		}
	}

	void Renderer::BuildGraph()
	{
		mRenderGraph = MakeRef<RenderGraph>();

		SamplerDescription depthSamplerDesc{};
		depthSamplerDesc.AddressModeU = SamplerAddressMode::ClampToEdge;
		depthSamplerDesc.AddressModeV = SamplerAddressMode::ClampToEdge;
		depthSamplerDesc.AddressModeW = SamplerAddressMode::ClampToEdge;
		depthSamplerDesc.BorderColor = SamplerBorderColor::White;

		ResourceHandle shadowDepthSampler = mResourceBuilder.CreateSampler("Sampler.Shadow.Depth", depthSamplerDesc);

		// Uniform Buffers
		ResourceHandle cameraBuffer = mResourceBuilder.CreateUniformBuffer("Buffer.Camera", sizeof(GPU::Camera));
		ResourceHandle directionalLightBuffer = mResourceBuilder.CreateUniformBuffer("Buffer.DirectionalLight", sizeof(GPU::DirectionalLight));
		ResourceHandle pointLightBuffer = mResourceBuilder.CreateUniformBuffer("Buffer.PointLight", sizeof(GPU::PointLightArray));
		ResourceHandle spotLightBuffer = mResourceBuilder.CreateUniformBuffer("Buffer.SpotLights", sizeof(GPU::SpotLightArray));
		ResourceHandle shadowDepthLightCameras = mResourceBuilder.CreateUniformBuffer("Buffer.Depth.LightCameras", sizeof(GPU::CascadedShadowLightMatrices));

		// Render Targets
		ResourceHandle gBufferAlbedo = mResourceBuilder.CreateTexture2D("GBuffer.Albedo", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferPosition = mResourceBuilder.CreateTexture2D("GBuffer.Position", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferNormal = mResourceBuilder.CreateTexture2D("GBuffer.Normal", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferPBRFactor = mResourceBuilder.CreateTexture2D("GBuffer.PBRFactor", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferDepth = mResourceBuilder.CreateTexture2D("GBuffer.Depth", TextureFormat::D_32F, TextureFlags::DepthAttachment);
		ResourceHandle shadowDepthTexture = mResourceBuilder.CreateTexture2DArray("Shadow.Depth", 4, TextureFormat::D_32F, TextureFlags::DepthAttachment);

		ResourceHandle mainOutput = mResourceBuilder.CreateTexture2D("MainOutput", TextureFormat::RGBA_32F, TextureFlags::RenderTarget | TextureFlags::StorageImage);

		// Shader ResourceGroups
		ResourceHandle cameraShaderResourceGroup = mResourceBuilder.CreateSRG("SRG.Camera", {
			ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Vertex)
			});

		ResourceHandle lightShaderResourceGroup = mResourceBuilder.CreateSRG("SRG.Lighting", {
			ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Compute), // Directional Light Buffer
			ShaderResourceDescription(1, ShaderResourceType::UniformBuffer, ShaderStage::Compute), // Point Light Buffer
			ShaderResourceDescription(2, ShaderResourceType::UniformBuffer, ShaderStage::Compute), // Spot Light Buffer
			});

		ResourceHandle lightingGBufferShaderResourceGroup = mResourceBuilder.CreateSRG("SRG.Lighting.GBuffer", {
			ShaderResourceDescription(0, ShaderResourceType::StorageImage, ShaderStage::Compute),
			ShaderResourceDescription(1, ShaderResourceType::Sampler, ShaderStage::Compute),
			ShaderResourceDescription(2, ShaderResourceType::Sampler, ShaderStage::Compute),
			ShaderResourceDescription(3, ShaderResourceType::Sampler, ShaderStage::Compute),
			ShaderResourceDescription(4, ShaderResourceType::Sampler, ShaderStage::Compute),
			});

		ResourceHandle lightingCameraShaderResourceGroup = mResourceBuilder.CreateSRG("SRG.Lighting.Camera", {
			ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Compute)
			});

		ResourceHandle skyboxEnvironmentMapShaderResourceGroup = mResourceBuilder.CreateSRG("SRG.Skybox.Environment", {
			ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Fragment),
			});

		ResourceHandle shadowDepthLightSpaceMatrices = mResourceBuilder.CreateSRG("SRG.Depth.LightCameras", {
			ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Geometry),
			});

		ResourceHandle lightingPassShadowSRG = mResourceBuilder.CreateSRG("SRG.Lighting.Shadow", {
			ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Compute),
			ShaderResourceDescription(1, ShaderResourceType::UniformBuffer, ShaderStage::Compute)
			});

		ResourceHandle lihgtingPassIBLSRG = mResourceBuilder.CreateSRG("SRG.lighting.IBL", {
			ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Compute),
			ShaderResourceDescription(1, ShaderResourceType::Sampler, ShaderStage::Compute),
			ShaderResourceDescription(2, ShaderResourceType::Sampler, ShaderStage::Compute),
			});


		// GBuffer Pass
		{
			WeakRef<GraphicsPipeline> gBufferPipeline = ShaderFactory::Get().GetOrCreateGraphicsPipeline("Geometry");
			WeakRef<RenderPass> GBufferPass = mRenderGraph->CreatePass("GBuffer Pass", PassType::Graphics);
			GBufferPass->AddCommandType(RenderCommandType::Draw);
			GBufferPass->AddResource(gBufferAlbedo, ResourceAccess::Write, 0);
			GBufferPass->AddResource(gBufferPosition, ResourceAccess::Write, 1);
			GBufferPass->AddResource(gBufferNormal, ResourceAccess::Write, 2);
			GBufferPass->AddResource(gBufferPBRFactor, ResourceAccess::Write, 3);
			GBufferPass->AddResource(gBufferDepth, ResourceAccess::Write);
			GBufferPass->AddResource(cameraShaderResourceGroup, ResourceAccess::Read, 0);
			GBufferPass->AddResource(mBindlessTextureSRGHandle, ResourceAccess::Read, 1);
			GBufferPass->AddResource(mBindlessMaterialSRGHandle, ResourceAccess::Read, 2);
			GBufferPass->SetPipeline(gBufferPipeline);

			GBufferPass->SetExecutionCallback([=](Ref<CommandBuffer> cmd, const CommandList& commandList, const ResourceRegistry& registry, uint32_t frameIndex) {
				// This isn't necessary for this pass because we only accept one command type but it will be good to che kin the future
				for (const auto& command : commandList.GetCommands())
				{
					if (command.GetType() == RenderCommandType::Draw) {

						const DrawCommand& drawCommand = command.GetCommand<DrawCommand>();
						WeakRef<Material> material = drawCommand.Material;
						glm::mat4 transform = drawCommand.ModelMatrix;

						if (material && material->Transparent)
							return;

						uint32_t materialIndex = 0;
						if (drawCommand.Material)
							materialIndex = material->GlobalIndex;

						cmd->SetPushConstants(gBufferPipeline, ShaderStage::Vertex, &transform[0][0], sizeof(transform));
						cmd->SetPushConstants(gBufferPipeline, ShaderStage::Fragment, &materialIndex, sizeof(materialIndex));
						cmd->BindAndDrawMesh(drawCommand.Mesh, 1);
					}
				}
				});
		}

		// Lighting Pass
		{
			WeakRef<ComputePipeline> lightingPassPipeline = ShaderFactory::Get().GetOrCreateComputePipeline("DeferredLighting");
			WeakRef<RenderPass> compositePass = mRenderGraph->CreatePass("Composite Pass", PassType::Compute);
			compositePass->AddResource(gBufferAlbedo, ResourceAccess::Read);
			compositePass->AddResource(gBufferPosition, ResourceAccess::Read);
			compositePass->AddResource(gBufferNormal, ResourceAccess::Read);
			compositePass->AddResource(gBufferPBRFactor, ResourceAccess::Read);
			compositePass->AddResource(shadowDepthTexture, ResourceAccess::Read);
			compositePass->AddResource(mainOutput, ResourceAccess::Write);
			compositePass->AddResource(lightingGBufferShaderResourceGroup, ResourceAccess::Read, 0);
			compositePass->AddResource(lightShaderResourceGroup, ResourceAccess::Read, 1);
			compositePass->AddResource(lightingCameraShaderResourceGroup, ResourceAccess::Read, 2);
			compositePass->AddResource(lightingPassShadowSRG, ResourceAccess::Read, 3);
			compositePass->AddResource(lihgtingPassIBLSRG, ResourceAccess::Read, 4);
			compositePass->SetPipeline(lightingPassPipeline);
			compositePass->SetExecutionCallback([=](Ref<CommandBuffer> cmd, const CommandList& command, const ResourceRegistry& registry, uint32_t frameIndex) {

				uint32_t width = registry.GetWidth(frameIndex) + 15;
				uint32_t height = registry.GetHeight(frameIndex) + 15;
				cmd->Execute(width / 16, height / 16, 1);
				});
		}

		// SkyBox Pass
		{
			WeakRef<GraphicsPipeline> environmentPipeline = ShaderFactory::Get().GetOrCreateGraphicsPipeline("EnvironmentMap");
			auto skyboxPass = mRenderGraph->CreatePass("Skybox", PassType::Graphics);
			skyboxPass->AddDependency("Composite Pass");
			skyboxPass->SetPipeline(environmentPipeline);
			skyboxPass->AddResource(mainOutput, ResourceAccess::Write, 0);
			skyboxPass->AddResource(gBufferDepth, ResourceAccess::Write); // Set this to write even though we dont so that it will get bound for testing
			skyboxPass->AddResource(cameraShaderResourceGroup, ResourceAccess::Read, 0);
			skyboxPass->AddResource(skyboxEnvironmentMapShaderResourceGroup, ResourceAccess::Read, 1);
			skyboxPass->SetExecutionCallback([=](Ref<CommandBuffer> cmd, const CommandList& commandList, const ResourceRegistry& registry, uint32_t frameIndex) {
				for (const auto& command : commandList.GetCommands())
				{
					if (command.GetType() != RenderCommandType::DrawSkyBox)
						continue;
			
					const auto& skyBoxCommand = command.GetCommand<DrawSkyboxCommand>();
			
					cmd->BindAndDrawMesh(skyBoxCommand.CubeMesh, 1);
				}
				});
		}

		// Depth Pass
		{
			WeakRef<GraphicsPipeline> depthPipeline = ShaderFactory::Get().GetOrCreateGraphicsPipeline("ShadowDepth");
			WeakRef<RenderPass> depthPass = mRenderGraph->CreatePass("Depth", PassType::Graphics);
			depthPass->SetPipeline(depthPipeline);
			depthPass->AddResource(shadowDepthLightSpaceMatrices, ResourceAccess::Read, 0);
			depthPass->AddResource(shadowDepthTexture, ResourceAccess::Write, 0);
			depthPass->SetExecutionCallback([=](Ref<CommandBuffer> cmd, const CommandList& commandList, const ResourceRegistry& registry, uint32_t frameIndex) {
				
				for (const auto& command : commandList.GetCommands())
				{
					if (command.GetType() != RenderCommandType::Draw)
						continue;

					const auto& drawCommand = command.GetCommand<DrawCommand>();

					glm::mat4 transform = drawCommand.ModelMatrix;
					cmd->SetPushConstants(depthPipeline, ShaderStage::Vertex, &transform[0][0], sizeof(transform));
					cmd->BindAndDrawMesh(drawCommand.Mesh, 1);
				}
				});
		}
				
		mRenderGraph->Bake();

		// Callbacks

		mRenderGraph->SetPreExecutionCallback([=](const Camera& camera, const CommandList& commandList, uint32_t frameIndex) {
			auto registry = camera.GetRegistry();
			
			auto skyboxSRG = registry->GetResource<ShaderResourceGroup>(skyboxEnvironmentMapShaderResourceGroup, frameIndex);

			auto cameraUB = registry->GetResource<UniformBuffer>(cameraBuffer, frameIndex);
			
			ScopedBuffer cameraBuffer = ScopedBuffer(sizeof(GPU::Camera));
			GPU::Camera* cameraBufferPtr = cameraBuffer.As<GPU::Camera>();
			cameraBufferPtr->ViewProjection = camera.GetViewProj();
			cameraBufferPtr->View = camera.GetView();
			cameraBufferPtr->Proj = camera.GetProj();
			cameraBufferPtr->Position = camera.GetPosition();
			cameraBufferPtr->ViewDirection = camera.GetForwardDir();
			
			cameraUB->SetData(cameraBuffer);

			ScopedBuffer directionalLightData(sizeof(GPU::DirectionalLight));
			ScopedBuffer pointLightData(sizeof(GPU::PointLightArray));
			ScopedBuffer spotLightData(sizeof(GPU::SpotLightArray));

			memset(directionalLightData.GetData(), 0, sizeof(GPU::DirectionalLight));
			memset(pointLightData.GetData(), 0, sizeof(GPU::PointLightArray));
			memset(spotLightData.GetData(), 0, sizeof(GPU::SpotLightArray));

			auto directionalLightUB = registry->GetResource<UniformBuffer>(directionalLightBuffer, frameIndex);
			auto pointLightUB = registry->GetResource<UniformBuffer>(pointLightBuffer, frameIndex);
			auto spotLightUB = registry->GetResource<UniformBuffer>(spotLightBuffer, frameIndex);

			glm::vec3 directionalLightDirection;
			for (const auto& command : commandList.GetCommands())
			{
				switch (command.GetType())
				{
				case RenderCommandType::DrawDirectionalLight:
				{
					const auto& directionalLight = command.GetCommand<DrawDirectionalLightCommand>();
					GPU::DirectionalLight* ptr = directionalLightData.As<GPU::DirectionalLight>();
					ptr->Direction = directionalLight.Direction;
					ptr->Color = directionalLight.Color;
					ptr->Intensity = directionalLight.Intensity;
					directionalLightDirection = directionalLight.Direction;
				}
					break;
				case RenderCommandType::DrawPointLight:
				{
					const auto& pointLight = command.GetCommand<DrawPointLightCommand>();
					GPU::PointLightArray* ptr = pointLightData.As<GPU::PointLightArray>();
					ptr->Lights[ptr->Count].Position = pointLight.Position;
					ptr->Lights[ptr->Count].Color = pointLight.Color;
					ptr->Lights[ptr->Count].Intensity = pointLight.Intensity;
					ptr->Count++;
				}
					break;
				case RenderCommandType::DrawSpotLight:
				{
					const auto& spotLight = command.GetCommand<DrawSpotLightCommand>();
					GPU::SpotLightArray* ptr = spotLightData.As<GPU::SpotLightArray>();
					ptr->Lights[ptr->Count].Color = spotLight.Color;
					ptr->Lights[ptr->Count].Position = spotLight.Position;
					ptr->Lights[ptr->Count].Direction = spotLight.Direction;
					ptr->Lights[ptr->Count].HalfAngle = spotLight.HalfAngle;
					ptr->Lights[ptr->Count].Intensity = spotLight.Intensity;
					ptr->Lights[ptr->Count].FallOff = spotLight.FallOff;
					ptr->Count++;
				}
					break;

				case RenderCommandType::DrawSkyBox:
				{
					const auto& skyBoxCommand = command.GetCommand<DrawSkyboxCommand>();
					skyboxSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)skyBoxCommand.SkyBox);

					auto lightpassIBLSRG = registry->GetResource<ShaderResourceGroup>(lihgtingPassIBLSRG, frameIndex);
					lightpassIBLSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)skyBoxCommand.DiffuseIBL);
					lightpassIBLSRG->Update(1, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)skyBoxCommand.PreFilterIBL);
					lightpassIBLSRG->Update(2, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)skyBoxCommand.BRDF);
				}
					break;
				}
			}

			directionalLightUB->SetData(directionalLightData);
			pointLightUB->SetData(pointLightData);
			spotLightUB->SetData(spotLightData);


			ScopedBuffer lightCameraBuffer(sizeof(GPU::CascadedShadowLightMatrices));
			memset(lightCameraBuffer.GetData(), 0, sizeof(GPU::CascadedShadowLightMatrices));
			GPU::CascadedShadowLightMatrices* lightCameraPtr = lightCameraBuffer.As<GPU::CascadedShadowLightMatrices>();
			
			Camera::CascadeSplits cascades = camera.GenerateLightSpaceCascades(4, directionalLightDirection);
			for (uint32_t i = 0; i < cascades.Count; i++)
			{
				lightCameraPtr->LightSpaceMatrices[i] = cascades.LightSpaceMatrices[i];
				lightCameraPtr->CascadeSplits[i].x = cascades.SplitDistances[i];
			}
			lightCameraPtr->CascadeCount = cascades.Count;

			auto shadowDepthLightCameraUB = registry->GetResource<UniformBuffer>(shadowDepthLightCameras, frameIndex);
			shadowDepthLightCameraUB->SetData(lightCameraBuffer);

			});

		mRenderGraph->SetResizeCallback([=](const Camera& camera, uint32_t frameIndex, uint32_t width, uint32_t height) {
			const ResourceRegistry& registry = *camera.GetRegistry();

			Ref<Texture2D> gMainOutput = registry.GetResource<Texture>(mainOutput, frameIndex);
			Ref<Texture2D> gAlbedo = registry.GetResource<Texture>(gBufferAlbedo, frameIndex);
			Ref<Texture2D> gPosition = registry.GetResource<Texture>(gBufferPosition, frameIndex);
			Ref<Texture2D> gNormal = registry.GetResource<Texture>(gBufferNormal, frameIndex);
			Ref<Texture2D> gPBR = registry.GetResource<Texture>(gBufferPBRFactor, frameIndex);
			Ref<Texture2D> gDepth = registry.GetResource<Texture>(gBufferDepth, frameIndex);

			gMainOutput->Resize(width, height);
			gAlbedo->Resize(width, height);
			gPosition->Resize(width, height);
			gNormal->Resize(width, height);
			gPBR->Resize(width, height);
			gDepth->Resize(width, height);

			auto lightingGBufferSRG = registry.GetResource<ShaderResourceGroup>(lightingGBufferShaderResourceGroup, frameIndex);
			lightingGBufferSRG->Update(0, DescriptorType::StorageImage, ImageLayout::General, (WeakRef<Texture>)gMainOutput);
			lightingGBufferSRG->Update(1, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gAlbedo);
			lightingGBufferSRG->Update(2, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gPosition);
			lightingGBufferSRG->Update(3, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gNormal);
			lightingGBufferSRG->Update(4, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gPBR);
			});

		mRenderGraph->SetRegistrySetupCallback([=](const ResourceRegistry& registry, uint32_t frameIndex) {

			// Deferred Pass Setup
			auto cameraUB = registry.GetResource<UniformBuffer>(cameraBuffer, frameIndex);
			auto cameraSRG = registry.GetResource<ShaderResourceGroup>(cameraShaderResourceGroup, frameIndex);

			cameraSRG->Update(0, cameraUB);


			// Lighting Pass Setup
			auto lightSRG = registry.GetResource<ShaderResourceGroup>(lightShaderResourceGroup, frameIndex);
			auto directionalLightUB = registry.GetResource<UniformBuffer>(directionalLightBuffer, frameIndex);
			auto pointLightUB = registry.GetResource<UniformBuffer>(pointLightBuffer, frameIndex);
			auto spotLightUB = registry.GetResource<UniformBuffer>(spotLightBuffer, frameIndex);

			lightSRG->Update(0, directionalLightUB);
			lightSRG->Update(1, pointLightUB);
			lightSRG->Update(2, spotLightUB);

			auto lightingGBufferSRG = registry.GetResource<ShaderResourceGroup>(lightingGBufferShaderResourceGroup, frameIndex);
			auto gMainOutput = registry.GetResource<Texture>(mainOutput, frameIndex);
			auto gAlbedo = registry.GetResource<Texture>(gBufferAlbedo, frameIndex);
			auto gPosition = registry.GetResource<Texture>(gBufferPosition, frameIndex);
			auto gNormal = registry.GetResource<Texture>(gBufferNormal, frameIndex);
			auto gPBR = registry.GetResource<Texture>(gBufferPBRFactor, frameIndex);
			
			lightingGBufferSRG->Update(0, DescriptorType::StorageImage, ImageLayout::General, (WeakRef<Texture>)gMainOutput);
			lightingGBufferSRG->Update(1, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gAlbedo);
			lightingGBufferSRG->Update(2, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gPosition);
			lightingGBufferSRG->Update(3, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gNormal);
			lightingGBufferSRG->Update(4, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)gPBR);

			auto lightingCameraSRG = registry.GetResource<ShaderResourceGroup>(lightingCameraShaderResourceGroup, frameIndex);
			lightingCameraSRG->Update(0, cameraUB);
			

			// Shadow Mapping
			auto lightCameraSRG = registry.GetResource<ShaderResourceGroup>(shadowDepthLightSpaceMatrices, frameIndex);
			auto lightCameraBuffer = registry.GetResource<UniformBuffer>(shadowDepthLightCameras, frameIndex);
			lightCameraSRG->Update(0, lightCameraBuffer);

			WeakRef<Texture2DArray> shadowDepthBuffer = registry.GetResource<Texture>(shadowDepthTexture, frameIndex);
			shadowDepthBuffer->Resize(2048, 2048);

			auto depthSampler = registry.GetResource<Sampler>(shadowDepthSampler, frameIndex);
			auto lightingShadowSRG = registry.GetResource<ShaderResourceGroup>(lightingPassShadowSRG, frameIndex);
			lightingShadowSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)shadowDepthBuffer, 0, depthSampler);
			lightingShadowSRG->Update(1, lightCameraBuffer);
			});
	}

	void Renderer::UpdateBindlessResources()
	{
		std::lock_guard<std::mutex> lock(mResourceMutex);

		auto bindlessTextureSRG = mBindlessTextureSRG[mFrameIndex];
		auto bindlessMaterialBuffer = mBindlessMaterialBuffer[mFrameIndex];

		auto getTextureIndex = [&](AssetHandle handle, WeakRef<Texture> defaultTexture) {

			if (handle == AssetHandle::Null())
				return mBindlessTextureIndices.QueryIndex(defaultTexture->Handle());

			uint32_t index = mBindlessTextureIndices.QueryIndex(handle);
			if (index == UINT32_MAX)
			{
				index = mBindlessTextureIndices.Insert(handle, defaultTexture);
				bindlessTextureSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, defaultTexture, index);
			}

			return index;
			};


		BindlessResourceUpdate& resourceUpdate = mResourceUpdates[mFrameIndex];

		for (auto texture : resourceUpdate.RemoveTextures)
			mBindlessTextureIndices.Remove(texture->Handle());

		for (auto texture : resourceUpdate.AddTextures)
		{
			// Materials may have been loaded first so we check the index to see if it exists

			uint32_t index = mBindlessTextureIndices.QueryIndex(texture->Handle());

			if (index == UINT32_MAX)
				index = mBindlessTextureIndices.Insert(texture->Handle(), texture);
			
			bindlessTextureSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, texture, index);
		}

		for (auto material : resourceUpdate.RemoveMaterials)
			mBindlessMaterialIndices.Remove(material->Handle());

		for (auto material : resourceUpdate.UpdateMaterials)
		{
			uint32_t index = mBindlessMaterialIndices.QueryIndex(material->Handle());
			if (index == UINT32_MAX)
			{
				resourceUpdate.AddMaterials.push_back(material);
				continue;
			}

			GPU::Material gpuMaterial = mBindlessMaterialIndices.Query(index);

			gpuMaterial.AlbedoColor = material->AlbedoColor;
			gpuMaterial.TextureScale = material->TextureScale;
			gpuMaterial.MetalnessFactor = material->MetalnessFactor;
			gpuMaterial.RoughnessFactor = material->RoughnessFactor;
			gpuMaterial.AOFactor = material->AOFactor;
			gpuMaterial.Transparency = material->Transparency;

			gpuMaterial.AlbedoIndex = getTextureIndex(material->AlbedoMap, mWhiteTex);
			gpuMaterial.NormalIndex = getTextureIndex(material->NormalMap, mDefaultNormalTex);
			gpuMaterial.MetalnessIndex = getTextureIndex(material->MetalnessMap, mWhiteTex);
			gpuMaterial.RoughnessIndex = getTextureIndex(material->RoughnessMap, mWhiteTex);
			gpuMaterial.AOIndex = getTextureIndex(material->AOMap, mWhiteTex);
			gpuMaterial.OpacityIndex = getTextureIndex(material->OpacityMap, mWhiteTex);
			gpuMaterial.EmissiveIndex = getTextureIndex(material->EmissiveMap, mBlackTex);

			mBindlessMaterialIndices.Update(index, gpuMaterial);
		}

		for (auto material : resourceUpdate.AddMaterials)
		{
			GPU::Material gpuMaterial;

			gpuMaterial.AlbedoColor = material->AlbedoColor;
			gpuMaterial.TextureScale = material->TextureScale;
			gpuMaterial.MetalnessFactor = material->MetalnessFactor;
			gpuMaterial.RoughnessFactor = material->RoughnessFactor;
			gpuMaterial.AOFactor = material->AOFactor;
			gpuMaterial.Transparency = material->Transparency;

			gpuMaterial.AlbedoIndex = getTextureIndex(material->AlbedoMap, mWhiteTex);
			gpuMaterial.NormalIndex = getTextureIndex(material->NormalMap, mDefaultNormalTex);
			gpuMaterial.MetalnessIndex = getTextureIndex(material->MetalnessMap, mWhiteTex);
			gpuMaterial.RoughnessIndex = getTextureIndex(material->RoughnessMap, mWhiteTex);
			gpuMaterial.AOIndex = getTextureIndex(material->AOMap, mWhiteTex);
			gpuMaterial.OpacityIndex = getTextureIndex(material->OpacityMap, mWhiteTex);
			gpuMaterial.EmissiveIndex = getTextureIndex(material->EmissiveMap, mBlackTex);

			uint32_t index = mBindlessMaterialIndices.QueryIndex(material->Handle());
			if (index == UINT32_MAX)
				index = mBindlessMaterialIndices.Insert(material->Handle(), gpuMaterial);

			material->GlobalIndex = index;
		}

		auto materialArray = mBindlessMaterialIndices.GetArray();
		Buffer materialData(materialArray.data(), materialArray.size() * sizeof(GPU::Material));
		bindlessMaterialBuffer->SetData(materialData);

		resourceUpdate.AddTextures.clear();
		resourceUpdate.RemoveTextures.clear();
		resourceUpdate.AddMaterials.clear();
		resourceUpdate.UpdateMaterials.clear();
		resourceUpdate.RemoveMaterials.clear();
	}
	
	bool Renderer::BindlessResourcesNeedUpdate()
	{
		if (!mResourceUpdates[mFrameIndex].AddTextures.empty())
			return true;

		if (!mResourceUpdates[mFrameIndex].RemoveTextures.empty())
			return true;

		if (!mResourceUpdates[mFrameIndex].AddMaterials.empty())
			return true;

		if (!mResourceUpdates[mFrameIndex].UpdateMaterials.empty())
			return true;

		if (!mResourceUpdates[mFrameIndex].RemoveMaterials.empty())
			return true;


		return false;
	}
}