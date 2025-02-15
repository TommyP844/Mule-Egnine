#include "Graphics/Scene Renderer/SceneRenderPass/ShadowPass.h"

#include "ECS/Components.h"

#include <glm/gtx/transform.hpp>

namespace Mule
{
	ShadowPass::ShadowPass(WeakRef<GraphicsContext> context, const ShadowPassInitInfo& initInfo)
		:
		ISceneRenderPass(2),
		mGraphicsContext(context),
		mAssetManager(initInfo.AssetManager)
	{
		RenderPassDescription renderPassDesc{};
		renderPassDesc.DepthAttachment = { TextureFormat::D32F };
		mRenderPass = mGraphicsContext->CreateRenderPass(renderPassDesc);

		mCommandPool = mGraphicsContext->GetGraphicsQueue()->CreateCommandPool();

		mFrameData.resize(2);
		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].ResizeRequired = true;
			mFrameData[i].ResizeWidth = 1024;
			mFrameData[i].ResizeHeight = 1024;

			mFrameData[i].ChangeCascadeCount = true;
			mFrameData[i].NewCascadeCount = 4;

			mFrameData[i].Fence = mGraphicsContext->CreateFence();
			mFrameData[i].Semaphore = mGraphicsContext->CreateSemaphore();
			mFrameData[i].CommandBuffer = mCommandPool->CreateCommandBuffer();
			mFrameData[i].CameraBuffer = mGraphicsContext->CreateUniformBuffer(sizeof(GPU::GPUCamera) * MAX_CASCADES);
		}

		VertexLayout staticVertexLayout;
		staticVertexLayout.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec2)
			.AddAttribute(AttributeType::Vec4);

		GraphicsShaderDescription shaderDesc{};
		shaderDesc.RenderPass = mRenderPass;
		shaderDesc.Subpass = 0;
		shaderDesc.SourcePath = "../Assets/Shaders/Graphics/ShadowDepthShader.glsl";
		shaderDesc.VertexLayout = staticVertexLayout;
		shaderDesc.PushConstants = {
			PushConstant(ShaderStage::Vertex, sizeof(glm::mat4))
		};

		mShader = mGraphicsContext->CreateGraphicsShader(shaderDesc);
	}

	ShadowPass::~ShadowPass()
	{
	}

	void ShadowPass::Render(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores, Camera camera)
	{
		auto& frameData = mFrameData[GetFrameIndex()];

		if (frameData.ResizeRequired || frameData.ChangeCascadeCount)
		{
			frameData.ResizeRequired = false;
			frameData.Width = frameData.ResizeWidth;
			frameData.Height = frameData.ResizeHeight;

			frameData.ChangeCascadeCount = false;
			frameData.CascadeCount = frameData.NewCascadeCount;

			FramebufferDescription framebufferDesc{};
			framebufferDesc.DepthAttachment = { TextureFormat::D32F };
			framebufferDesc.Width = frameData.Width;
			framebufferDesc.Height = frameData.Height;
			framebufferDesc.LayerCount = 1;
			framebufferDesc.RenderPass = mRenderPass;

			DescriptorSetLayoutDescription DSLD{
				.Layouts = {
					LayoutDescription(0, DescriptorType::UniformBuffer, ShaderStage::Geometry)
				}
			};

			frameData.FrameBuffers.resize(frameData.CascadeCount);
			for (uint32_t j = 0; j < frameData.CascadeCount; j++)
			{
				frameData.FrameBuffers[j] = mGraphicsContext->CreateFrameBuffer(framebufferDesc);
			}

			frameData.CascadeDistances.resize(frameData.CascadeCount + 1);
			frameData.LightCameras.resize(frameData.CascadeCount + 1);
		}
		

		bool foundLight = false;
		glm::vec3 direction;
		for (auto id : scene->Iterate<DirectionalLightComponent>())
		{
			Entity e((uint32_t)id, scene);
			const DirectionalLightComponent& directionalLight = e.GetComponent<DirectionalLightComponent>();
			if (!directionalLight.Active) continue;
			foundLight = true;
			glm::quat rotation = glm::quat(glm::radians(e.GetTransformComponent().Rotation));
			direction = rotation * glm::vec4(0, -1, 0, 0);
			direction = glm::normalize(direction);
			break;
		}

		if (!foundLight)
		{
			frameData.DidRender = false;
			return;
		}

		frameData.DidRender = true;

		frameData.Fence->Wait();
		frameData.Fence->Reset();
				
		float clipDistance = (camera.GetFarPlane() - camera.GetNearPlane()) / static_cast<float>(frameData.CascadeCount);
		float nearClip = camera.GetNearPlane();
		float farClip = camera.GetFarPlane();
		float lambda = 0.75f;

		for (int i = 0; i <= frameData.CascadeCount; i++) {
			float fi = float(i) / float(frameData.CascadeCount);
			float logSplit = nearClip * powf(farClip / nearClip, fi);
			float linearSplit = nearClip + (farClip - nearClip) * fi;
			frameData.CascadeDistances[i] = lambda * logSplit + (1.0f - lambda) * linearSplit;
		}

		for (uint32_t i = 0; i < frameData.CascadeCount; i++)
		{
			nearClip = frameData.CascadeDistances[i];
			farClip = frameData.CascadeDistances[i + 1];
			std::array<glm::vec3, 8> frustumCorners = GetFrustumCornersWorldSpace(camera.GetView(), nearClip, farClip, camera.GetAspectRatio(), camera.GetFOVDegrees());

			glm::vec3 center = glm::vec3(0);
			for (const auto& corner : frustumCorners) {
				center += corner;
			}
			center /= 8.0f;

			// make sure direction is NOT {0, -1, 0} or glm::lookAt returns garbage
			glm::mat4 lightView = glm::lookAt(center, center + direction, glm::vec3(0, 1, 0));

			float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::lowest();
			float minY = std::numeric_limits<float>::max(), maxY = std::numeric_limits<float>::lowest();
			float minZ = std::numeric_limits<float>::max(), maxZ = std::numeric_limits<float>::lowest();

			for (const auto& corner : frustumCorners) {
				glm::vec3 transformed = glm::vec3(lightView * glm::vec4(corner, 1.0f));
				minX = std::min(minX, transformed.x);
				maxX = std::max(maxX, transformed.x);
				minY = std::min(minY, transformed.y);
				maxY = std::max(maxY, transformed.y);
				minZ = std::min(minZ, transformed.z);
				maxZ = std::max(maxZ, transformed.z);
			}

			glm::mat4 lightProj = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

			frameData.LightCameras[i] = lightProj * lightView;
		}

		frameData.CameraBuffer->SetData(&frameData.LightCameras[0], sizeof(GPU::GPUCamera) * frameData.LightCameras.size());

		Ref<CommandBuffer> cmd = frameData.CommandBuffer;
		
		cmd->Reset();
		cmd->Begin();

		for (int i = 0; i < frameData.CascadeCount; i++)
		{
			cmd->TranistionImageLayout(frameData.FrameBuffers[i]->GetDepthAttachment(), ImageLayout::DepthAttachment);
			cmd->BeginRenderPass(frameData.FrameBuffers[i], mRenderPass);
			cmd->BindGraphicsPipeline(mShader);

			for (auto entityId : scene->Iterate<MeshComponent>())
			{
				Entity e((uint32_t)entityId, scene);
				const MeshComponent& meshComponent = e.GetComponent<MeshComponent>();
				if (!meshComponent.Visible) continue;
				if (!meshComponent.CastsShadows) continue;

				auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
				if (!mesh) continue;
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
				transform = frameData.LightCameras[i] * e.GetTransformComponent().TRS();

				cmd->SetPushConstants(mShader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
				cmd->BindAndDrawMesh(mesh, 1);
			}
			cmd->EndRenderPass();
			cmd->TranistionImageLayout(frameData.FrameBuffers[i]->GetDepthAttachment(), ImageLayout::ShaderReadOnly);
		}

		cmd->End();
		mGraphicsContext->GetGraphicsQueue()->Submit(cmd, {}, {frameData.Semaphore}, frameData.Fence);
	}

	void ShadowPass::Resize(uint32_t width, uint32_t height)
	{
		SPDLOG_WARN("Invalid function call, ShadowPass::Resize(uint32_t, uint32_t)");
	}

	void ShadowPass::SetCascadeCount(uint32_t count)
	{
		for (uint32_t i = 0; i < 2; i++)
		{
			mFrameData[i].ChangeCascadeCount = true;
			mFrameData[i].NewCascadeCount = count;
		}
	}

	void ShadowPass::SetCascadeSize(uint32_t width, uint32_t height)
	{
		for (uint32_t i = 0; i < 2; i++)
		{
			mFrameData[i].ResizeRequired = true;
			mFrameData[i].ResizeWidth = width;
			mFrameData[i].ResizeHeight = height;
		}
	}

	std::array<glm::vec3, 8> ShadowPass::GetFrustumCornersWorldSpace(const glm::mat4& view, float nearClip, float farClip, float fov, float aspectRatio)
	{
		std::array<glm::vec3, 8> corners;

		glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);

		glm::mat4 invVP = glm::inverse(proj * view);
		std::vector<glm::vec4> ndcCorners = {
			{ -1,  1, 0, 1 }, { 1,  1, 0, 1 },
			{ 1, -1, 0, 1 }, { -1, -1, 0, 1 },
			{ -1,  1, 1, 1 }, { 1,  1, 1, 1 },
			{ 1, -1, 1, 1 }, { -1, -1, 1, 1 }
		};

		for (int i = 0; i < 8; i++) {
			glm::vec4 corner = invVP * ndcCorners[i];
			corners[i] = glm::vec3(corner) / corner.w;
		}
		return corners;
	}
}