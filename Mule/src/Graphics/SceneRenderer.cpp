#include "Graphics/SceneRenderer.h"

#include "Graphics/VertexLayout.h"
#include "Graphics/Material.h"

#include "ECS/Components.h"

namespace Mule
{
	SceneRenderer::SceneRenderer(Ref<GraphicsContext> context, Ref<AssetManager> assetManager)
		:
		mGraphicsContext(context),
		mAssetManager(assetManager),
		mFrameIndex(0),
		mIsValid(true)
	{
		SPDLOG_INFO("Albedo Color Offset: {}", offsetof(GPUMaterial, AlbedoColor));
		SPDLOG_INFO("Texture Scale Offset: {}", offsetof(GPUMaterial, TextureScale));
		SPDLOG_INFO("Metalness Factor Offset: {}", offsetof(GPUMaterial, MetalnessFactor));
		SPDLOG_INFO("Roughness Factor Offset: {}", offsetof(GPUMaterial, RoughnessFactor));
		SPDLOG_INFO("AOFactor Offset: {}", offsetof(GPUMaterial, AOFactor));
		SPDLOG_INFO("AlbedoIndex Offset: {}", offsetof(GPUMaterial, AlbedoIndex));
		SPDLOG_INFO("NormalIndex Offset: {}", offsetof(GPUMaterial, NormalIndex));
		SPDLOG_INFO("MetalnessIndex Offset: {}", offsetof(GPUMaterial, MetalnessIndex));
		SPDLOG_INFO("RoughnessIndex Offset: {}", offsetof(GPUMaterial, RoughnessIndex));
		SPDLOG_INFO("AOIndex Offset: {}", offsetof(GPUMaterial, AOIndex));
		SPDLOG_INFO("EmissiveIndex Offset: {}", offsetof(GPUMaterial, EmissiveIndex));

		VertexLayout staticVertexLayout;
		staticVertexLayout.AddAttribute(AttributeType::Vec3);
		staticVertexLayout.AddAttribute(AttributeType::Vec3);
		staticVertexLayout.AddAttribute(AttributeType::Vec3);
		staticVertexLayout.AddAttribute(AttributeType::Vec2);
		staticVertexLayout.AddAttribute(AttributeType::Vec4);

		RenderPassDescription geometryRenderPassDesc{};
		geometryRenderPassDesc.Attachments = {
			{ TextureFormat::RGBA8U }
		};
		geometryRenderPassDesc.DepthAttachment = { TextureFormat::D32F };
		mMainRenderPass = context->CreateRenderPass(geometryRenderPassDesc);

		FramebufferDescription framebufferDescription{};
		framebufferDescription.RenderPass = mMainRenderPass;
		framebufferDescription.Width = 800;
		framebufferDescription.Height = 600;
		framebufferDescription.LayerCount = 1;
		framebufferDescription.Attachments = {
			{ TextureFormat::RGBA8U }
		};
		framebufferDescription.DepthAttachment = {
			TextureFormat::D32F
		};


		DescriptorSetLayoutDescription descriptorSetLayout{};

		LayoutDescription descriptorLayoutDesc;
		descriptorLayoutDesc.ArrayCount = 1;
		descriptorLayoutDesc.Binding = 0;
		descriptorLayoutDesc.Stage = ShaderStage::Vertex;
		descriptorLayoutDesc.Type = DescriptorType::UniformBuffer;
		descriptorSetLayout.Layouts.push_back(descriptorLayoutDesc);

		descriptorLayoutDesc.ArrayCount = 4096;
		descriptorLayoutDesc.Binding = 1;
		descriptorLayoutDesc.Stage = ShaderStage::Fragment;
		descriptorLayoutDesc.Type = DescriptorType::Texture;
		descriptorSetLayout.Layouts.push_back(descriptorLayoutDesc);

		descriptorLayoutDesc.ArrayCount = 1;
		descriptorLayoutDesc.Binding = 2;
		descriptorLayoutDesc.Stage = ShaderStage::Fragment;
		descriptorLayoutDesc.Type = DescriptorType::UniformBuffer;
		descriptorSetLayout.Layouts.push_back(descriptorLayoutDesc);

		mGeometryStageLayout = mGraphicsContext->CreateDescriptorSetLayout(descriptorSetLayout);

		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].RenderingFinishedFence = mGraphicsContext->CreateFence();
			mFrameData[i].Framebuffer = mGraphicsContext->CreateFrameBuffer(framebufferDescription);
			mFrameData[i].Framebuffer->SetColorClearValue(0, glm::vec4(0, 0, 0, 0));
			mFrameData[i].RenderingFinishedSemaphore = mGraphicsContext->CreateSemaphore();
			mFrameData[i].CommandPool = mGraphicsContext->GetGraphicsQueue()->CreateCommandPool();
			mFrameData[i].SolidGeometryPassCmdBuffer = mFrameData[i].CommandPool->CreateCommandBuffer();
			mFrameData[i].CameraBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(CameraData));

			// TODO: handle dynamic material buffer size
			mFrameData[i].MaterialBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPUMaterial) * 100);

			DescriptorSetDescription descriptorDesc{};
			descriptorDesc.Layouts = { mGeometryStageLayout };
			mFrameData[i].DescriptorSet = mGraphicsContext->CreateDescriptorSet(descriptorDesc);

			DescriptorSetUpdate updateCameraBuffer;
			updateCameraBuffer.ArrayElement = 0;
			updateCameraBuffer.Binding = 0;
			updateCameraBuffer.Type = DescriptorType::UniformBuffer;
			updateCameraBuffer.Buffers = { mFrameData[i].CameraBuffer };

			DescriptorSetUpdate updateMaterialBuffer;
			updateMaterialBuffer.ArrayElement = 0;
			updateMaterialBuffer.Binding = 2;
			updateMaterialBuffer.Type = DescriptorType::UniformBuffer;
			updateMaterialBuffer.Buffers = { mFrameData[i].MaterialBuffer };

			mFrameData[i].DescriptorSet->Update({ updateCameraBuffer, updateMaterialBuffer });
		}

		GraphicsShaderDescription defaultGeometryDesc{};
		defaultGeometryDesc.SourcePath = "../Assets/Shaders/DefaultGeometryShader.glsl";
		defaultGeometryDesc.RenderPass = mMainRenderPass;
		defaultGeometryDesc.Subpass = 0;
		defaultGeometryDesc.VertexLayout = staticVertexLayout;
		defaultGeometryDesc.DescriptorLayouts = { mGeometryStageLayout };
		defaultGeometryDesc.PushConstants = {
			PushConstant(ShaderStage::Vertex, 64),
			PushConstant(ShaderStage::Fragment, sizeof(uint32_t))
		};

		mDefaultGeometryShader = context->CreateGraphicsShader(defaultGeometryDesc);
	}
	
	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::OnRender(Ref<Scene> scene, std::vector<WeakRef<Semaphore>> waitSemaphore)
	{
		if (!mIsValid) return;
		mFrameIndex ^= 1;
		FrameData& frameData = mFrameData[mFrameIndex];

	}
	
	void SceneRenderer::OnEditorRender(const EditorRenderSettings& settings)
	{
		if (!mIsValid) return;
		mFrameIndex ^= 1;
		FrameData& frameData = mFrameData[mFrameIndex];

		frameData.RenderingFinishedFence->Wait();
		frameData.RenderingFinishedFence->Reset();

		frameData.CommandPool->Reset();

		if (frameData.ResizeRequired)
		{
			frameData.ResizeRequired = false;
			frameData.Framebuffer->Resize(frameData.ResizeWidth, frameData.ResizeHeight);
		}

		// Data Prep
		{

			CameraData cameraData;
			cameraData.View = settings.EditorCamera.GetView();
			cameraData.Proj = settings.EditorCamera.GetProj();

			frameData.CameraBuffer->SetData(&cameraData, sizeof(cameraData));

			frameData.TextureArray.Clear();
			frameData.MaterialArray.Clear();

			// TODO: so empty textures have somewhere to point
			// frameData.TextureArray.Insert(NullAssetHandle, nullptr);

			for (Ref<ITexture> texture : mAssetManager->GetAssetsOfType(AssetType::Texture))
			{
				uint32_t index = frameData.TextureArray.Insert(texture->Handle(), texture);
				//SPDLOG_INFO("Texture: {}, index: {}", texture->Name(), index);
			}

			// Update texture array descriptor
			DescriptorSetUpdate updateTextureBuffer;
			updateTextureBuffer.ArrayElement = 0;
			updateTextureBuffer.Binding = 1;
			updateTextureBuffer.Type = DescriptorType::Texture;
			
			for (auto texture : frameData.TextureArray.GetArray())
			{
				updateTextureBuffer.Textures.push_back(texture);
			}
			frameData.DescriptorSet->Update({ updateTextureBuffer });

			// TODO: so assets with no material have a default
			//frameData.MaterialArray.Insert(NullAssetHandle, GPUMaterial());

			for (Ref<Material> material : mAssetManager->GetAssetsOfType(AssetType::Material))
			{
				GPUMaterial gpuMaterial{};

				gpuMaterial.AlbedoColor = material->AlbedoColor;

				gpuMaterial.TextureScale = material->TextureScale;

				gpuMaterial.MetalnessFactor = material->MetalnessFactor;
				gpuMaterial.RoughnessFactor = material->RoughnessFactor;
				gpuMaterial.AOFactor = material->AOFactor;

				gpuMaterial.AlbedoIndex = frameData.TextureArray.QueryIndex(material->AlbedoMap);
				gpuMaterial.NormalIndex = frameData.TextureArray.QueryIndex(material->NormalMap);
				gpuMaterial.MetalnessIndex = frameData.TextureArray.QueryIndex(material->MetalnessMap);
				gpuMaterial.RoughnessIndex = frameData.TextureArray.QueryIndex(material->RoughnessMap);
				gpuMaterial.AOIndex = frameData.TextureArray.QueryIndex(material->AOMap);
				gpuMaterial.EmissiveIndex = frameData.TextureArray.QueryIndex(material->EmissiveMap);

				uint32_t index = frameData.MaterialArray.Insert(material->Handle(), gpuMaterial);

				//SPDLOG_INFO("Material: {}, index: {}", material->Name(), index);
			}


			frameData.MaterialBuffer->SetData(
				frameData.MaterialArray.GetArray().data(),
				frameData.MaterialArray.GetArray().size() * sizeof(GPUMaterial));

		}

		// Render
		{
			Ref<CommandBuffer> commandBuffer = frameData.SolidGeometryPassCmdBuffer;
			commandBuffer->Begin();
			commandBuffer->TranistionImageLayout(frameData.Framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
			commandBuffer->BeginRenderPass(frameData.Framebuffer, mMainRenderPass);

			commandBuffer->BindGraphicsPipeline(mDefaultGeometryShader);
			commandBuffer->BindDescriptorSet(mDefaultGeometryShader, frameData.DescriptorSet);

			settings.Scene->IterateEntitiesWithComponents<MeshComponent>([&](Entity e) {

				const auto& meshComponent = e.GetComponent<MeshComponent>();

				if (!meshComponent.Visible) return;

				auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
				if (!mesh) return;

				glm::mat4 transform = e.GetTransformComponent().TRS();
				uint32_t materialIndex = frameData.MaterialArray.QueryIndex(meshComponent.MaterialHandle);

				commandBuffer->SetPushConstants(mDefaultGeometryShader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				commandBuffer->SetPushConstants(mDefaultGeometryShader, ShaderStage::Fragment, &materialIndex, sizeof(uint32_t));
				commandBuffer->BindMesh(mesh);
				commandBuffer->DrawMesh(mesh);
				

				});

			commandBuffer->EndRenderPass();
			commandBuffer->TranistionImageLayout(frameData.Framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly);
			commandBuffer->End();

			mGraphicsContext->GetGraphicsQueue()->Submit(commandBuffer, {}, { frameData.RenderingFinishedSemaphore }, frameData.RenderingFinishedFence);
		}


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
}