#pragma once


// Engine
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Execution/CommandBuffer.h"
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

#ifdef min
#undef min
#endif

namespace Mule
{
	struct SceneRendererStats
	{
		float CPUPrepareTime = 0.f;
		float CPUExecutionTime = 0.f;
		std::vector<RenderGraph::RenderPassStats> RenderPassStats;
	};

	struct SceneRendererDebugOptions
	{
		bool ShowAllPhysicsObjects = false;
		bool ShowAllLights = false;
		
		Entity SelectedEntity = Entity();
		bool ShowSelectedEntityColliders = true;
		bool ShowSelectedEntityLights = true;
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

		bool OnEditorRender(WeakRef<Scene> scene, const Camera& camera, const std::vector<WeakRef<Semaphore>>& waitSemaphores);
		bool OnRender(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores);

		Guid Pick(uint32_t x, uint32_t y);

		WeakRef<Semaphore> GetSemaphore() const;
		WeakRef<FrameBuffer> GetFrameBuffer() const;

		void Resize(uint32_t width, uint32_t height);

		const SceneRendererStats& GetRenderStats() const { return mTiming[mGraph.GetFrameIndex()]; }
		SceneRendererDebugOptions& GetDebugOptions() { return mDebugOptions; }

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

		// Performance
		std::vector<SceneRendererStats> mTiming;

		// Debug
		SceneRendererDebugOptions mDebugOptions;

		void PrepareResources(const Camera& camera, WeakRef<Scene> scene);

		// Render Passes
		void RenderSolidGeometryCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx);
		void RenderTransparentGeometryCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx);
		void RenderEnvironmentCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx);
		void RenderEntityMaskCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx);
		void RenderEntityOutlineCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx);
		void RenderEditorBillboardCallback(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx);
		void RenderPhysicsUIPass(WeakRef<CommandBuffer> cmd, WeakRef<Scene> scene, WeakRef<GraphicsShader> shader, const RenderGraph::PassContext& ctx);

		// Render Graph Passes
		const std::string GEOMETRY_PASS_NAME = "OpaqueGeometryPass";
		const std::string ENVIRONMENT_PASS_NAME = "EnvironmentPass";
		const std::string TRANPARENT_GEOMETRY_PASS_NAME = "TransparentGeometryPass";
		const std::string ENTITY_HIGHLIGHT_PASS_NAME = "EntityHighlightPass";
		const std::string ENTTIY_OUTLINE_PASS_NAME = "EntityOutlinePass";
		const std::string EDITOR_BILLBOARD_PASS_NAME = "EditorUIPass";
		const std::string PHYSICS_UI_PASS_NAME = "PhysicsUIPass";

		// Descriptor Sets

		const std::string GEOMETRY_SHADER_DSL_ID = "GeometryShaderDSL";
		const std::string GEOMETRY_SHADER_DS_ID = "GeometryShaderDS";

		const std::string BINDLESS_TEXTURE_DSL_ID = "BindlessTextureDSL";
		const std::string BINDLESS_TEXTURE_DS_ID = "BindlessTextureDS";

		const std::string ENVIRONMENT_SHADER_DSL_ID = "EnvironmentShaderDSL";
		const std::string ENVIRONMENT_SHADER_DS_ID = "EnvironmentShaderDS";

		const std::string ENTITY_OUTLINE_DSL_ID = "EntityOutlineDSL";
		const std::string ENTITY_OUTLINE_DS_ID = "EntityOutlineDS";

		const std::string ENTITY_MASK_DSL_ID = "EntityMaskDSL";
		const std::string ENTITY_MASK_DS_ID = "EntityMaskDS";

		const std::string BILLBOARD_DSL_ID = "BillboardDSL";
		const std::string BILLBOARD_DS_ID = "BillboardDS";

		const std::string WIREFRAME_DSL_ID = "WireframeDSL";
		const std::string WIREFRAME_DS_ID = "WireframeDS";

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