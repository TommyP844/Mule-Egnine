#include "Graphics/SceneRenderer.h"

#include "Graphics/VertexLayout.h"

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

		mGeometryStageLayout = mGraphicsContext->CreateDescriptorSetLayout(descriptorSetLayout);

		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].RenderingFinishedFence = mGraphicsContext->CreateFence();
			mFrameData[i].Framebuffer = mGraphicsContext->CreateFrameBuffer(framebufferDescription);
			mFrameData[i].Framebuffer->SetColorClearValue(0, glm::vec4(1, 0, 0, 1));
			mFrameData[i].RenderingFinishedSemaphore = mGraphicsContext->CreateSemaphore();
			mFrameData[i].CommandPool = mGraphicsContext->GetGraphicsQueue()->CreateCommandPool();
			mFrameData[i].SolidGeometryPassCmdBuffer = mFrameData[i].CommandPool->CreateCommandBuffer();
			mFrameData[i].CameraBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(CameraData));

			DescriptorSetDescription descriptorDesc{};
			descriptorDesc.Layouts = { mGeometryStageLayout };
			mFrameData[i].DescriptorSet = mGraphicsContext->CreateDescriptorSet(descriptorDesc);

			DescriptorSetUpdate updateBuffer;
			updateBuffer.ArrayElement = 0;
			updateBuffer.Binding = 0;
			updateBuffer.Type = DescriptorType::UniformBuffer;
			updateBuffer.Buffers = { mFrameData[i].CameraBuffer };

			mFrameData[i].DescriptorSet->Update({ updateBuffer });
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

		}

		// Render
		{
			Ref<CommandBuffer> commandBuffer = frameData.SolidGeometryPassCmdBuffer;
			commandBuffer->Begin();
			commandBuffer->TranistionImageLayout(frameData.Framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
			commandBuffer->BeginRenderPass(frameData.Framebuffer, mMainRenderPass);

			commandBuffer->BindPipeline(mDefaultGeometryShader);

			scene->IterateEntitiesWithComponents<MeshCollectionComponent>([&](Entity e) {

				const auto& meshCollection = e.GetComponent<MeshCollectionComponent>();
				for (const auto& meshComponent : meshCollection.Meshes)
				{
					if (!meshComponent.Visible) continue;

					auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
					if (!mesh) continue;
					
					commandBuffer->BindMesh(mesh);
					commandBuffer->DrawMesh(mesh);
				}

				});

			commandBuffer->EndRenderPass();
			commandBuffer->TranistionImageLayout(frameData.Framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly);
			commandBuffer->End();

			mGraphicsContext->GetGraphicsQueue()->Submit(commandBuffer, {}, { frameData.RenderingFinishedSemaphore }, frameData.RenderingFinishedFence);
		}


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

		}

		// Render
		{
			Ref<CommandBuffer> commandBuffer = frameData.SolidGeometryPassCmdBuffer;
			commandBuffer->Begin();
			commandBuffer->TranistionImageLayout(frameData.Framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
			commandBuffer->BeginRenderPass(frameData.Framebuffer, mMainRenderPass);

			commandBuffer->BindPipeline(mDefaultGeometryShader);
			commandBuffer->BindDescriptorSet(mDefaultGeometryShader, frameData.DescriptorSet);

			settings.Scene->IterateEntitiesWithComponents<MeshCollectionComponent>([&](Entity e) {

				const auto& meshCollection = e.GetComponent<MeshCollectionComponent>();
				for (const auto& meshComponent : meshCollection.Meshes)
				{
					if (!meshComponent.Visible) continue;

					auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
					if (!mesh) continue;

					glm::mat4 transform = e.GetTransformComponent().TRS();
					uint32_t materialIndex;

					commandBuffer->SetPushConstants(mDefaultGeometryShader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
					commandBuffer->SetPushConstants(mDefaultGeometryShader, ShaderStage::Fragment, &materialIndex, sizeof(uint32_t));
					commandBuffer->BindMesh(mesh);
					commandBuffer->DrawMesh(mesh);
				}

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