#pragma once

// Engine
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Texture/TextureCube.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Material.h"
#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderPassStats.h"
#include "GuidArray.h"
#include "GPUObjects.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "Asset/AssetManager.h"

// STD
#include <vector>

namespace Mule
{
	struct SceneRendererStats
	{
		float CPUPrepareTime = 0.f;
		float CPUExecutionTime = 0.f;
		std::vector<RenderGraph::RenderPassStats> RenderPassStats;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer(Ref<GraphicsContext> context, Ref<AssetManager> assetManager);
		~SceneRenderer();

		void RefreshEngineObjects();

		void AddTexture(WeakRef<ITexture> texture);
		void RemoveTexture(AssetHandle textureHandle);

		void AddMaterial(WeakRef<Material> material);
		void UpdateMaterial(WeakRef<Material> material);
		void RemoveMaterial(AssetHandle materialHandle);

		void OnEditorRender(WeakRef<Scene> scene, const Camera& camera, const std::vector<WeakRef<Semaphore>>& waitSemaphores);
		void OnRender(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores);

		Guid Pick(uint32_t x, uint32_t y);

		WeakRef<Semaphore> GetSemaphore() const;
		WeakRef<FrameBuffer> GetFrameBuffer() const;

		void Resize(uint32_t width, uint32_t height);

		const SceneRendererStats& GetRenderStats() const { return mTiming[mGraph.GetFrameIndex()]; }

	private:
		std::mutex mMutex;

		Ref<GraphicsContext> mGraphicsContext;
		Ref<AssetManager> mAssetManager;

		uint32_t mWhiteImageIndex;
		uint32_t mBlackImageIndex;
		uint32_t mBlackCubeMapImageIndex;

		GuidArray<WeakRef<ITexture>> mTextureArray;
		GuidArray<GPU::GPUMaterial> mMaterialArray;

		RenderGraph::RenderGraph mGraph;

		uint32_t QueryOrInsertTextureIndex(AssetHandle handle, uint32_t defaultIndex);

		// Timing
		std::vector<SceneRendererStats> mTiming;

		// Render Passes
		void RenderSolidGeometryCallback(const RenderGraph::PassContext& ctx);
		void RenderTransparentGeometryCallback(const RenderGraph::PassContext& ctx);
		void RenderEnvironmentCallback(const RenderGraph::PassContext& ctx);
		void RenderEntityHighlightCallback(const RenderGraph::PassContext& ctx);
		bool RenderEntityChildrenHighlight(Entity e, WeakRef<CommandBuffer> cmd, WeakRef<GraphicsShader> shader);

		// Render Graph Passes
		const std::string GEOMETRY_PASS_NAME = "OpaqueGeometryPass";
		const std::string ENVIRONMENT_PASS_NAME = "EnvironmentPass";
		const std::string TRANPARENT_GEOMETRY_PASS_NAME = "TransparentGeometryPass";
		const std::string ENTITY_HIGHLIGHT_PASS_NAME = "EntityHighlightPass";

		// Resources
		const std::string FRAMEBUFFER_ID = "Framebuffer";
		const std::string GEOMETRY_RENDER_PASS_ID = "GeometryRenderPass";

		// Shaders
		const std::string OPAQUE_GEOMETRY_SHADER_ID = "OpaqueGeometryShader";
		const std::string TRANPARENT_GEOMETRY_SHADER_ID = "TransparentGeometryShader";
		const std::string ENVIRONMENT_SHADER_ID = "EnvironmentShader";
		const std::string ENTITY_HIGHLIGHT_SHADER_ID = "EntityHighlightShader";
		const std::string ENTITY_OUTLINE_SHADER_ID = "EntityOutlineShader";

		// Descriptor Sets
		const std::string GEOMETRY_SHADER_DSL_ID = "GeometryShaderDSL";
		const std::string GEOMETRY_SHADER_DS_ID = "GeometryShaderDS";

		const std::string BINDLESS_TEXTURE_DSL_ID = "BindlessTextureDSL";
		const std::string BINDLESS_TEXTURE_DS_ID = "BindlessTextureDS";

		const std::string ENVIRONMENT_SHADER_DSL_ID = "EnvironmentShaderDSL";
		const std::string ENVIRONMENT_SHADER_DS_ID = "EnvironmentShaderDS";

		const std::string ENTITY_HIGHLIGHT_DSL_ID = "EntityHighlightDSL";
		const std::string ENTITY_HIGHLIGHT_DS_ID = "EntityHighlightDS";

		const std::string ENTITY_OUTLINE_DSL_ID = "EntityOutlineDSL";
		const std::string ENTITY_OUTLINE_DS_ID = "EntityOutlineDS";

		// Buffers
		const std::string CAMERA_BUFFER_ID = "CameraUB";
		const std::string LIGHT_BUFFER_ID = "LightUB";
		const std::string MATERIAL_BUFFER_ID = "MaterialUB";


		// Updates for textures/materials
		struct ResourceUpdate
		{
			std::vector<std::pair<WeakRef<ITexture>, uint32_t>> TextureUpdates;
			std::vector<std::pair<GPU::GPUMaterial, uint32_t>> MaterialUpdates;
			bool Resize = false;
			uint32_t ResizeWidth, ResizeHeight;
		};
		std::vector<ResourceUpdate> mResourceUpdates;
	};
}