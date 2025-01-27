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
		
		RenderPassDescription geometryRenderPassDesc{};

		geometryRenderPassDesc.Attachments = {
			{ TextureFormat::RGBA8U }
		};
		geometryRenderPassDesc.DepthAttachment = { TextureFormat::D32F };

		mMainRenderPass = context->CreateRenderPass(geometryRenderPassDesc);

		GraphicsShaderDescription defaultGeometryDesc{};
		defaultGeometryDesc.SourcePath = "../Assets/Shaders/DefaultGeometryShader.glsl";
		defaultGeometryDesc.RenderPass = mMainRenderPass;
		defaultGeometryDesc.Subpass = 0;
		defaultGeometryDesc.VertexLayout = staticVertexLayout;

		mDefaultGeometryShader = context->CreateGraphicsShader(defaultGeometryDesc);

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

		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].RenderingFinishedFence = mGraphicsContext->CreateFence();
			mFrameData[i].Framebuffer = mGraphicsContext->CreateFrameBuffer(framebufferDescription);
			mFrameData[i].Framebuffer->SetColorClearValue(0, glm::vec4(1, 0, 0, 1));
			mFrameData[i].RenderingFinishedSemaphore = mGraphicsContext->CreateSemaphore();
			mFrameData[i].CommandPool = mGraphicsContext->GetGraphicsQueue()->CreateCommandPool();
			mFrameData[i].ShadowPassCmdBuffer = mFrameData[i].CommandPool->CreateCommandBuffer();
			mFrameData[i].SolidGeometryPassCmdBuffer = mFrameData[i].CommandPool->CreateCommandBuffer();
		}
	}
	
	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::Render(Ref<Scene> scene, std::vector<Ref<Semaphore>> waitSemaphore)
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

		// Render
		{
			Ref<CommandBuffer> commandBuffer = frameData.SolidGeometryPassCmdBuffer;
			commandBuffer->Begin();
			commandBuffer->TranistionImageLayout(frameData.Framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
			commandBuffer->BeginRenderPass(frameData.Framebuffer, mMainRenderPass);

			// render

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