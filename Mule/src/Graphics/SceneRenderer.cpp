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
			mFrameData[i].Framebuffer = mGraphicsContext->CreateFrameBuffer(framebufferDescription);
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

		// Shadow Pass
		{

		}

		GeometryPass(scene);

	}

	void SceneRenderer::GeometryPass(Ref<Scene> scene)
	{
		FrameData& frameData = mFrameData[mFrameIndex];
		// TODO: Eventually this will need to be more complex because if we have to many materials in our scene
		// we will need multiple buffers and to preform multiple draw operation and switch descriptor sets

		struct MeshMetaData
		{
			AssetHandle MaterialHandle;
			glm::mat4 Transform;
		};

		struct ShaderMaterial
		{
			glm::vec4 AlbedoColor;
			float Metalness;
			float Roughness;
			float AO;
			float EMissive;
			uint32_t AlbedoIndex;
			uint32_t NormalIndex;
			uint32_t MetalnessIndex;
			uint32_t RoughnessIndex;
			uint32_t AOIndex;
			uint32_t EmissiveIndex;
		};


		std::map<AssetHandle, std::vector<MeshMetaData>> meshMaterials;
		scene->IterateEntitiesWithComponents<MeshComponent>([&](Entity e) {
			TransformComponent& transform = e.GetComponent<TransformComponent>();
			MeshComponent& meshComponent = e.GetComponent<MeshComponent>();
			if (meshComponent.Visible)
				return;
			if (meshComponent.MeshHandle == NullAssetHandle)
				return;

			// TODO: handle fustum culling here

			MeshMetaData metaData{};
			metaData.MaterialHandle = meshComponent.MaterialHandle ? meshComponent.MaterialHandle : 0;
			metaData.Transform = glm::mat4(1.f);

			meshMaterials[meshComponent.MeshHandle].push_back(metaData);

			});

		std::vector<ShaderMaterial> shaderMaterials;
		std::vector<glm::mat4> transforms;
		std::map<AssetHandle, std::pair<uint32_t, uint32_t>> instancedMeshOffsets;


		uint32_t offset = 0;
		for (const auto& [meshHandle, meshData] : meshMaterials)
		{
			// TODO: sort meshData from nearest to furthest to prevent over draw

			for (auto& md : meshData)
			{
				transforms.push_back(md.Transform); 

				// TODO: grab material data from asset manager and populate shadermaterial array
				// also we need an array of textures and a way to map there indices
				ShaderMaterial shaderMaterial;

				shaderMaterials.push_back(shaderMaterial);
			}

			instancedMeshOffsets[meshHandle] = { offset, meshData.size() };
			offset += meshData.size();
		}

		//Update uniform buffers
		// 
		// Update descriptor sets

		frameData.SolidGeometryPassCmdBuffer->BindPipeline(mDefaultGeometryShader);
		// TODO: bind descriptor set

		for (const auto& [meshHandle, data] : instancedMeshOffsets)
		{
			WeakRef<Mesh> mesh = mAssetManager->GetAsset<Mesh>(meshHandle);
			if (!mesh)
			{
				SPDLOG_WARN("Mesh handle invalid in draw call: {}", meshHandle);
				continue;
			}

			uint32_t offset = data.first;
			frameData.SolidGeometryPassCmdBuffer->SetPushConstants(mDefaultGeometryShader, ShaderStage::Vertex, offset);
			frameData.SolidGeometryPassCmdBuffer->SetPushConstants(mDefaultGeometryShader, ShaderStage::Fragment, offset);
			frameData.SolidGeometryPassCmdBuffer->BindAndDrawMesh(mesh, data.second);
		}
	}
}