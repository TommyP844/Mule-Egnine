#include "Graphics/SceneRenderer.h"

#include "Graphics/VertexLayout.h"
#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"

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

		descriptorLayoutDesc.ArrayCount = 1;
		descriptorLayoutDesc.Binding = 2;
		descriptorLayoutDesc.Stage = ShaderStage::Fragment;
		descriptorLayoutDesc.Type = DescriptorType::UniformBuffer;
		descriptorSetLayout.Layouts.push_back(descriptorLayoutDesc);

		mGeometryStageLayout = mGraphicsContext->CreateDescriptorSetLayout(descriptorSetLayout);

#pragma region EnvironmentMap

		Buffer environmentMeshVertices;
		Buffer environmentMeshIndices;

		// Allocate memory for 8 vertices (each vertex is a glm::vec3)
		environmentMeshVertices.Allocate(sizeof(glm::vec3) * 8);

		// Define the 8 vertices of the unit cube centered around the origin
		environmentMeshVertices.As<glm::vec3>()[0] = glm::vec3(-0.5f, -0.5f, -0.5f) * 10.f; // Bottom-left-back
		environmentMeshVertices.As<glm::vec3>()[1] = glm::vec3(0.5f, -0.5f, -0.5f) * 10.f; // Bottom-right-back
		environmentMeshVertices.As<glm::vec3>()[2] = glm::vec3(0.5f, 0.5f, -0.5f) * 10.f; // Top-right-back
		environmentMeshVertices.As<glm::vec3>()[3] = glm::vec3(-0.5f, 0.5f, -0.5f) * 10.f; // Top-left-back
		environmentMeshVertices.As<glm::vec3>()[4] = glm::vec3(-0.5f, -0.5f, 0.5f) * 10.f; // Bottom-left-front
		environmentMeshVertices.As<glm::vec3>()[5] = glm::vec3(0.5f, -0.5f, 0.5f) * 10.f; // Bottom-right-front
		environmentMeshVertices.As<glm::vec3>()[6] = glm::vec3(0.5f, 0.5f, 0.5f) * 10.f; // Top-right-front
		environmentMeshVertices.As<glm::vec3>()[7] = glm::vec3(-0.5f, 0.5f, 0.5f) * 10.f; // Top-left-front

		// Allocate memory for 36 indices (12 triangles * 3 indices each, using uint16_t)
		environmentMeshIndices.Allocate(sizeof(uint16_t) * 3 * 2 * 6);

		// Define the indices for the 12 triangles (clockwise order)
		uint16_t* indices = environmentMeshIndices.As<uint16_t>();

		// Front face
		indices[0] = 4; indices[1] = 5; indices[2] = 6; // Triangle 1
		indices[3] = 4; indices[4] = 6; indices[5] = 7; // Triangle 2

		// Back face
		indices[6] = 0; indices[7] = 3; indices[8] = 2;  // Triangle 1
		indices[9] = 0; indices[10] = 2; indices[11] = 1; // Triangle 2

		// Left face
		indices[12] = 0; indices[13] = 4; indices[14] = 7; // Triangle 1
		indices[15] = 0; indices[16] = 7; indices[17] = 3; // Triangle 2

		// Right face
		indices[18] = 1; indices[19] = 2; indices[20] = 6; // Triangle 1
		indices[21] = 1; indices[22] = 6; indices[23] = 5; // Triangle 2

		// Bottom face
		indices[24] = 0; indices[25] = 1; indices[26] = 5; // Triangle 1
		indices[27] = 0; indices[28] = 5; indices[29] = 4; // Triangle 2

		// Top face
		indices[30] = 3; indices[31] = 7; indices[32] = 6; // Triangle 1
		indices[33] = 3; indices[34] = 6; indices[35] = 2; // Triangle 2

		MeshDescription environmentMeshDesc{};
		environmentMeshDesc.Name = "Environment Cube";
		environmentMeshDesc.Vertices = environmentMeshVertices;
		environmentMeshDesc.VertexSize = sizeof(glm::vec3);
		environmentMeshDesc.Indices = environmentMeshIndices;
		environmentMeshDesc.IndexBufferType = IndexBufferType::BufferSize_16Bit;

		mEnvironmentCube = MakeRef<Mesh>(mGraphicsContext, environmentMeshDesc);

		environmentMeshVertices.Release();
		environmentMeshIndices.Release();

		DescriptorSetLayoutDescription environmentSetLayout{};

		LayoutDescription environmentLayoutDesc;
		environmentLayoutDesc.ArrayCount = 1;
		environmentLayoutDesc.Binding = 0;
		environmentLayoutDesc.Stage = ShaderStage::Fragment;
		environmentLayoutDesc.Type = DescriptorType::Texture;
		environmentSetLayout.Layouts.push_back(environmentLayoutDesc);

		environmentLayoutDesc.ArrayCount = 1;
		environmentLayoutDesc.Binding = 1;
		environmentLayoutDesc.Stage = ShaderStage::Vertex;
		environmentLayoutDesc.Type = DescriptorType::UniformBuffer;
		environmentSetLayout.Layouts.push_back(environmentLayoutDesc);

		mEnvironmentMapDescriptorSetLayout = mGraphicsContext->CreateDescriptorSetLayout(environmentSetLayout);

		VertexLayout environmentLayout;
		environmentLayout.AddAttribute(AttributeType::Vec3);

		GraphicsShaderDescription environmentShaderdesc{};
		environmentShaderdesc.SourcePath = "../Assets/Shaders/Graphics/EnvironmentMapShader.glsl";
		environmentShaderdesc.RenderPass = mMainRenderPass;
		environmentShaderdesc.Subpass = 0;
		environmentShaderdesc.VertexLayout = environmentLayout;
		environmentShaderdesc.DescriptorLayouts = { mEnvironmentMapDescriptorSetLayout };
		environmentShaderdesc.PushConstants = {};

		mEnvironmentMapShader = context->CreateGraphicsShader(environmentShaderdesc);

#pragma endregion

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

			DescriptorSetDescription environmentDesc{};
			environmentDesc.Layouts = { mEnvironmentMapDescriptorSetLayout };
			mFrameData[i].EnvironmentMapDescriptorSet = mGraphicsContext->CreateDescriptorSet(environmentDesc);

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
		defaultGeometryDesc.SourcePath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
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
			commandBuffer->BindGraphicsDescriptorSet(mDefaultGeometryShader, frameData.DescriptorSet);

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

			SkyLightComponent skyLight;
			settings.Scene->IterateEntitiesWithComponents<SkyLightComponent>([&](Entity e) {

				const auto& skyLightComponent = e.GetComponent<SkyLightComponent>();
				if (skyLightComponent.Active && skyLightComponent.EnvironmentMap != NullAssetHandle)
				{
					skyLight = skyLightComponent;
					return;
				}

				});

			auto environmentMap = mAssetManager->GetAsset<EnvironmentMap>(skyLight.EnvironmentMap);
			if (environmentMap)
			{
				auto cubeMap = mAssetManager->GetAsset<TextureCube>(environmentMap->GetCubeMapHandle());

				if (cubeMap)
				{
					DescriptorSetUpdate environmentUpdate{};
					environmentUpdate.ArrayElement = 0;
					environmentUpdate.Binding = 0;
					environmentUpdate.Textures = { cubeMap };
					environmentUpdate.Type = DescriptorType::Texture;

					DescriptorSetUpdate environmentUpdate1{};
					environmentUpdate1.ArrayElement = 0;
					environmentUpdate1.Binding = 1;
					environmentUpdate1.Buffers = { frameData.CameraBuffer };
					environmentUpdate1.Type = DescriptorType::UniformBuffer;

					frameData.EnvironmentMapDescriptorSet->Update({ environmentUpdate, environmentUpdate1 });

					commandBuffer->BindGraphicsPipeline(mEnvironmentMapShader);
					commandBuffer->BindGraphicsDescriptorSet(mEnvironmentMapShader, frameData.EnvironmentMapDescriptorSet);
					commandBuffer->BindAndDrawMesh(mEnvironmentCube, 1);
				}
			}

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