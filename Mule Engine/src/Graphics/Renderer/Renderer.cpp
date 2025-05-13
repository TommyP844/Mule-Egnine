
#include "Graphics/Renderer/Renderer.h"

#include "Graphics/GPUObjects.h"
#include "Graphics/ShaderFactory.h"

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
			environmentMapPipeline.EnableDepthTest = false;
			environmentMapPipeline.EnableDepthWrite = false;
			shaderFactory.RegisterGraphicsPipeline("EnvironmentMap", environmentMapPipeline);
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

		registry->SetOutputHandle(ResourceHandle("GBuffer.Albedo", ResourceType::RenderTarget));

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

		for (auto renderRequest : mRenderRequests)
		{
			auto registry = renderRequest.Camera.GetRegistry();
			if (!registry)
				continue;
			registry->WaitForFences(mFrameIndex);
		}

		UpdateBindlessResources();

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

		// Uniform Buffers
		ResourceHandle cameraBuffer = mResourceBuilder.CreateUniformBuffer("CameraBuffer", sizeof(GPU::Camera));

		// Render Targets
		ResourceHandle gBufferAlbedo = mResourceBuilder.CreateTexture2D("GBuffer.Albedo", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferPosition = mResourceBuilder.CreateTexture2D("GBuffer.Position", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferNormal = mResourceBuilder.CreateTexture2D("GBuffer.Normal", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferPBRFactor = mResourceBuilder.CreateTexture2D("GBuffer.PBRFactor", TextureFormat::RGBA_32F, TextureFlags::RenderTarget);
		ResourceHandle gBufferDepth = mResourceBuilder.CreateTexture2D("GBuffer.Depth", TextureFormat::D_32F, TextureFlags::DepthAttachment);

		ResourceHandle mainOutput = mResourceBuilder.CreateTexture2D("MainOutput", TextureFormat::RGBA_8U, TextureFlags::RenderTarget);

		// Shader ResourceGroups
		ResourceHandle cameraShaderResourceGroup = mResourceBuilder.CreateSRG("Camera.SRG", {
			ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Vertex)
			});

		// Render Passes
		WeakRef<GraphicsPipeline> gBufferPipeline = ShaderFactory::Get().GetOrCreateGraphicsPipeline("Geometry");
		WeakRef<RenderPass> GBufferPass = mRenderGraph->CreatePass("GBuffer Pass", PassType::Graphics);
		GBufferPass->AddCommandType(RenderCommandType::Draw);
		GBufferPass->AddResource(gBufferAlbedo, ResourceAccess::Write, 0);
		GBufferPass->AddResource(gBufferPosition, ResourceAccess::Write, 1);
		GBufferPass->AddResource(gBufferNormal, ResourceAccess::Write, 2);
		GBufferPass->AddResource(gBufferPBRFactor, ResourceAccess::Write, 3);
		GBufferPass->AddResource(cameraShaderResourceGroup, ResourceAccess::Read, 0);
		GBufferPass->AddResource(mBindlessTextureSRGHandle, ResourceAccess::Read, 1);
		GBufferPass->AddResource(mBindlessMaterialSRGHandle, ResourceAccess::Read, 2);
		GBufferPass->SetPipeline(gBufferPipeline);
		GBufferPass->SetExecutionCallback([=](Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex) {
			// This isnt nessesary for this pass because we only accept one command type but it will be good to che kin the future
			if (command.GetType() == RenderCommandType::Draw) {
				WeakRef<GraphicsPipeline> pipeline = ShaderFactory::Get().GetOrCreateGraphicsPipeline("Geometry");

				const DrawCommand& drawCommand = command.GetCommand<DrawCommand>();
				WeakRef<Material> material = drawCommand.Material;
				glm::mat4 transform = drawCommand.ModelMatrix;

				if (material && material->Transparent)
					return;

				uint32_t materialIndex = 0;
				if (drawCommand.Material)
					materialIndex = material->GlobalIndex;

				cmd->SetPushConstants(pipeline, ShaderStage::Vertex, &transform[0][0], sizeof(transform));
				cmd->SetPushConstants(pipeline, ShaderStage::Fragment, &materialIndex, sizeof(materialIndex));
				cmd->BindAndDrawMesh(drawCommand.Mesh, 1);
			}
			});


		WeakRef<RenderPass> compositePass = mRenderGraph->CreatePass("Composite Pass", PassType::Compute);
		compositePass->AddResource(gBufferAlbedo, ResourceAccess::Read);
		compositePass->AddResource(gBufferPosition, ResourceAccess::Read);
		compositePass->AddResource(gBufferNormal, ResourceAccess::Read);
		compositePass->AddResource(gBufferPBRFactor, ResourceAccess::Read);
		
		
				
		mRenderGraph->Bake();

		// Callbacks

		mRenderGraph->SetPreExecutionCallback([=](const Camera& camera, uint32_t frameIndex) {
			auto registry = camera.GetRegistry();
			
			auto uniformBuffer = registry->GetResource<UniformBuffer>(cameraBuffer, frameIndex);
			auto cameraSRG = registry->GetResource<ShaderResourceGroup>(cameraShaderResourceGroup, frameIndex);

			cameraSRG->Update(0, uniformBuffer);
			
			Buffer cameraBuffer = Buffer(sizeof(GPU::Camera));
			GPU::Camera* cameraBufferPtr = cameraBuffer.As<GPU::Camera>();
			cameraBufferPtr->ViewProjection = camera.GetViewProj();
			cameraBufferPtr->View = camera.GetView();
			cameraBufferPtr->Position = camera.GetPosition();
			cameraBufferPtr->ViewDirection = camera.GetForwardDir();
			
			uniformBuffer->SetData(cameraBuffer);
			});

		mRenderGraph->SetResizeCallback([=](const Camera& camera, uint32_t frameIndex, uint32_t width, uint32_t height) {
			const ResourceRegistry& registry = *camera.GetRegistry();

			registry.GetResource<Texture2D>(gBufferAlbedo, frameIndex)->Resize(width, height);
			registry.GetResource<Texture2D>(gBufferPosition, frameIndex)->Resize(width, height);
			registry.GetResource<Texture2D>(gBufferNormal, frameIndex)->Resize(width, height);
			registry.GetResource<Texture2D>(gBufferPBRFactor, frameIndex)->Resize(width, height);
			registry.GetResource<Texture2D>(gBufferDepth, frameIndex)->Resize(width, height);
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
				bindlessTextureSRG->Update(0, DescriptorType::Texture, defaultTexture, index);
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
			
			bindlessTextureSRG->Update(0, DescriptorType::Texture, texture, index);
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
}