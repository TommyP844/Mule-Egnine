#include "Graphics/Scene Renderer/SceneRenderPass/GeometryPass.h"

#include "ECS/Components.h"
#include "Graphics/Material.h"
#include "Engine Context/EngineAssets.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/Texture/TextureCube.h"
#include "Graphics/Texture/Texture2D.h"

namespace Mule
{
	GeometryPass::GeometryPass(WeakRef<GraphicsContext> context, const GeometryPassInitInfo& initInfo)
		:
		ISceneRenderPass(2),
		mGraphicsContext(context),
		mTextureArray(initInfo.TextureArray),
		mMaterialArray(initInfo.MaterialArray),
		mAssetManager(initInfo.AssetManager)
	{
		DescriptorSetLayoutDescription descriptorSetLayout;
		descriptorSetLayout.Layouts = {
			LayoutDescription(0, DescriptorType::UniformBuffer, (ShaderStage)((uint32_t)ShaderStage::Vertex | (uint32_t)ShaderStage::Fragment)),	// Camera Buffer
			LayoutDescription(1, DescriptorType::UniformBuffer, ShaderStage::Fragment),																// Material Buffer
			LayoutDescription(2, DescriptorType::UniformBuffer, ShaderStage::Fragment),																// Light Buffer
			LayoutDescription(4, DescriptorType::Texture, ShaderStage::Fragment),																	// Irradiance Map
			LayoutDescription(5, DescriptorType::Texture, ShaderStage::Fragment),																	// Pre Filter Cube Map
			LayoutDescription(6, DescriptorType::Texture, ShaderStage::Fragment),																	// BRDF Lut
		};
		mGeometryDescriptorSetLayout = mGraphicsContext->CreateDescriptorSetLayout(descriptorSetLayout);

		DescriptorSetLayoutDescription environmentSetLayout;
		environmentSetLayout.Layouts = {
			LayoutDescription(0, DescriptorType::Texture, ShaderStage::Fragment),
			LayoutDescription(1, DescriptorType::UniformBuffer, ShaderStage::Vertex)
		};
		mEnvironmentDescriptorSetLayout = mGraphicsContext->CreateDescriptorSetLayout(environmentSetLayout);

#pragma region Frame Data

		RenderPassDescription geometryRenderPassDesc{
			.Attachments = {
				{ TextureFormat::RGBA32F }
			},
			.DepthAttachment = { TextureFormat::D32F }
		};
		mRenderPass = context->CreateRenderPass(geometryRenderPassDesc);

		FramebufferDescription framebufferDescription{
			.Width = 800,
			.Height = 600,
			.LayerCount = 1,
			.RenderPass = mRenderPass,
			.Attachments = {
				{ TextureFormat::RGBA32F }
			},
			.DepthAttachment = { TextureFormat::D32F }
		};

		mCommandPool = mGraphicsContext->GetGraphicsQueue()->CreateCommandPool();

		mFrameData.resize(mFrameCount);
		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			mFrameData[i].Fence = mGraphicsContext->CreateFence();
			mFrameData[i].Semaphore = mGraphicsContext->CreateSemaphore();
			mFrameData[i].CommandBuffer = mCommandPool->CreateCommandBuffer();
			mFrameData[i].FrameBuffer = mGraphicsContext->CreateFrameBuffer(framebufferDescription);
			mFrameData[i].FrameBuffer->SetColorClearValue(0, glm::vec4(0));
			mFrameData[i].BindlessTextureDS = initInfo.FrameInfo[i].BindlessTextureDS;

			DescriptorSetDescription descriptorSetDesc{
				.Layouts = { mGeometryDescriptorSetLayout }
			};
			
			mFrameData[i].GeometryDescriptorSet = mGraphicsContext->CreateDescriptorSet(descriptorSetDesc);
			
			std::vector<DescriptorSetUpdate> geometryDSU = {
				DescriptorSetUpdate(0, DescriptorType::UniformBuffer, 0, {}, { initInfo.FrameInfo[i].CameraBuffer }),	// Camera Buffer
				DescriptorSetUpdate(1, DescriptorType::UniformBuffer, 0, {}, { initInfo.FrameInfo[i].MaterialBuffer }),	// Material Buffer
				DescriptorSetUpdate(2, DescriptorType::UniformBuffer, 0, {}, { initInfo.FrameInfo[i].LightBuffer }),	// Light Buffer
			};

			mFrameData[i].GeometryDescriptorSet->Update(geometryDSU);

			DescriptorSetDescription environmentSetDesc{
				.Layouts = { mEnvironmentDescriptorSetLayout }
			};
			mFrameData[i].EnvironmentDS = mGraphicsContext->CreateDescriptorSet(environmentSetDesc);
			std::vector<DescriptorSetUpdate> environmentDSU = {
				DescriptorSetUpdate(1, DescriptorType::UniformBuffer, 0, {}, { initInfo.FrameInfo[i].CameraBuffer })
			};

			mFrameData[i].EnvironmentDS->Update(environmentDSU);
		}

#pragma endregion


		VertexLayout staticVertexLayout;
		staticVertexLayout.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec3)
			.AddAttribute(AttributeType::Vec2)
			.AddAttribute(AttributeType::Vec4);

		GraphicsShaderDescription geometryShaderDesc{};
		geometryShaderDesc.DescriptorLayouts = { mGeometryDescriptorSetLayout, initInfo.FrameInfo[0].BindlessTextureDS->GetDescriptorSetLayouts()[0] };
		geometryShaderDesc.RenderPass = mRenderPass;
		geometryShaderDesc.Subpass = 0;
		geometryShaderDesc.SourcePath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
		geometryShaderDesc.VertexLayout = staticVertexLayout;
		geometryShaderDesc.PushConstants = {
			PushConstant(ShaderStage::Vertex, sizeof(glm::mat4)),
			PushConstant(ShaderStage::Fragment, sizeof(uint32_t) * 2)
		};

		mOpaqueGeometryShader = mGraphicsContext->CreateGraphicsShader(geometryShaderDesc);

		GraphicsShaderDescription environmentShaderDesc{};
		environmentShaderDesc.DescriptorLayouts = { mEnvironmentDescriptorSetLayout };
		environmentShaderDesc.RenderPass = mRenderPass;
		environmentShaderDesc.Subpass = 0;
		environmentShaderDesc.SourcePath = "../Assets/Shaders/Graphics/EnvironmentMapShader.glsl";
		environmentShaderDesc.VertexLayout = staticVertexLayout;
		environmentShaderDesc.PushConstants = {};
		environmentShaderDesc.CulleMode = CullMode::Front;

		mEnvironmentMapShader = mGraphicsContext->CreateGraphicsShader(environmentShaderDesc);
	}
	
	GeometryPass::~GeometryPass()
	{
	}

	void GeometryPass::Render(const GeometryPassRenderInfo& renderInfo)
	{
		FrameData& frameData = mFrameData[GetFrameIndex()];

		if (frameData.ResizeRequired)
		{
			frameData.ResizeRequired = false;
			frameData.FrameBuffer->Resize(frameData.ResizeWidth, frameData.ResizeHeight);
		}

		WeakRef<EnvironmentMap> envMap = nullptr;
		for (auto entityId : renderInfo.Scene->Iterate<EnvironmentMapComponent>())
		{
			Entity e((uint32_t)entityId, renderInfo.Scene);
			const EnvironmentMapComponent& environmentMap = e.GetComponent<EnvironmentMapComponent>();
			if (!environmentMap.Active) continue;

			envMap = mAssetManager->GetAsset<EnvironmentMap>(environmentMap.EnvironmentMap);
			break;
		}

		if (envMap)
		{
			auto cubeMap = mAssetManager->GetAsset<TextureCube>(envMap->GetCubeMapHandle());
			if(!cubeMap)
				cubeMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);

			auto irradianceMap = mAssetManager->GetAsset<TextureCube>(envMap->GetDiffuseIBLMap());
			if (!irradianceMap)
				irradianceMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);

			auto prefilterMap = mAssetManager->GetAsset<TextureCube>(envMap->GetPreFilterMap());
			if (!prefilterMap)
				prefilterMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);

			auto brdfLut = mAssetManager->GetAsset<Texture2D>(envMap->GetBRDFLutMap());
			if (!brdfLut)
				brdfLut = mAssetManager->GetAsset<Texture2D>(MULE_BLACK_TEXTURE_HANDLE);

			std::vector<DescriptorSetUpdate> geometryDSU = {
				DescriptorSetUpdate(4, DescriptorType::Texture, 0, { irradianceMap }, {}),	// Irradiance Map 
				DescriptorSetUpdate(5, DescriptorType::Texture, 0, { prefilterMap }, {}),	// Pre-Filter Map
				DescriptorSetUpdate(6, DescriptorType::Texture, 0, { brdfLut }, {}),		// BRDF Lut Map
			};

			frameData.GeometryDescriptorSet->Update(geometryDSU);

			std::vector<DescriptorSetUpdate> environmentDSU = {
				DescriptorSetUpdate(0, DescriptorType::Texture, 0, { cubeMap }, {})
			};
			frameData.EnvironmentDS->Update(environmentDSU);
		}
		else
		{
			auto irradianceMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);
			auto prefilterMap = mAssetManager->GetAsset<TextureCube>(MULE_BLACK_TEXTURE_CUBE_HANDLE);
			auto brdfLut = mAssetManager->GetAsset<Texture2D>(MULE_BLACK_TEXTURE_HANDLE);

			std::vector<DescriptorSetUpdate> geometryDSU = {
				DescriptorSetUpdate(4, DescriptorType::Texture, 0, { irradianceMap }, {}),	// Irradiance Map 
				DescriptorSetUpdate(5, DescriptorType::Texture, 0, { prefilterMap }, {}),	// Pre-Filter Map
				DescriptorSetUpdate(6, DescriptorType::Texture, 0, { brdfLut }, {}),		// BRDF Lut Map
			};

			frameData.GeometryDescriptorSet->Update(geometryDSU);
		}

		Ref<CommandBuffer> cmd = frameData.CommandBuffer;

		cmd->Reset();
		cmd->Begin();
		cmd->TranistionImageLayout(frameData.FrameBuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
		cmd->BeginRenderPass(frameData.FrameBuffer, mRenderPass);

		cmd->BindGraphicsPipeline(mOpaqueGeometryShader);
		cmd->BindGraphicsDescriptorSet(mOpaqueGeometryShader, { frameData.GeometryDescriptorSet, frameData.BindlessTextureDS });

		for (auto entityId : renderInfo.Scene->Iterate<MeshComponent>())
		{
			Entity e((uint32_t)entityId, renderInfo.Scene);

			const MeshComponent& meshComponent = e.GetComponent<MeshComponent>();
			if (!meshComponent.Visible) continue;

			auto mesh = mAssetManager->GetAsset<Mesh>(meshComponent.MeshHandle);
			if (!mesh) continue;

			uint32_t materialIndex = mMaterialArray.QueryIndex(meshComponent.MaterialHandle);
			if (materialIndex == UINT32_MAX)
				materialIndex = 0;

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
				renderInfo.LightCameras.size()
			};

			cmd->SetPushConstants(mOpaqueGeometryShader, ShaderStage::Vertex, &transform[0][0], sizeof(glm::mat4));
			cmd->SetPushConstants(mOpaqueGeometryShader, ShaderStage::Fragment, &fragmentConstants[0], sizeof(uint32_t) * 2);

			cmd->BindAndDrawMesh(mesh, 1);
		}

		if (envMap)
		{
			cmd->BindGraphicsPipeline(mEnvironmentMapShader);
			cmd->BindGraphicsDescriptorSet(mEnvironmentMapShader, { frameData.EnvironmentDS });
			auto mesh = mAssetManager->GetAsset<Mesh>(MULE_CUBE_MESH_HANDLE);
			cmd->BindAndDrawMesh(mesh, 1);
		}

		cmd->EndRenderPass();
		cmd->TranistionImageLayout(frameData.FrameBuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly);
		cmd->End();

		mGraphicsContext->GetGraphicsQueue()->Submit(cmd, renderInfo.WaitSemaphores, { frameData.Semaphore }, frameData.Fence);
	}

	void GeometryPass::Resize(uint32_t width, uint32_t height)
	{
		for (uint32_t i = 0; i < 2; i++)
		{
			mFrameData[i].ResizeRequired = true;
			mFrameData[i].ResizeWidth = width;
			mFrameData[i].ResizeHeight = height;
		}
	}

}