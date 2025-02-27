#pragma once

// Engine
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Texture/TextureCube.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Material.h"
#include "ECS/Scene.h"
#include "Asset/AssetManager.h"
#include "GPUObjects.h"
#include "GuidArray.h"
#include "Graphics/RenderGraph/RenderGraph.h"

// STD
#include <vector>

namespace Mule
{
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

		WeakRef<Semaphore> GetSemaphore() const;
		WeakRef<FrameBuffer> GetFrameBuffer() const;

		void Resize(uint32_t width, uint32_t height);

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

		void PrepareDrawData(const RenderGraph::PassContext& ctx);
		void RenderSolidGeometryCallback(const RenderGraph::PassContext& ctx);
		void RenderTransparentGeometryCallback(const RenderGraph::PassContext& ctx);
		void RenderEnvironmentCallback(const RenderGraph::PassContext& ctx);
		void RenderEntityHighlightCallback(const RenderGraph::PassContext& ctx);

		// Resources
		const std::string FRAMEBUFFER_ID = "Framebuffer";
		const std::string GEOMETRY_RENDER_PASS_ID = "GeometryPass";

		// Shaders
		const std::string GEOMETRY_SHADER_ID = "GeometryShader";
		const std::string ENVIRONMENT_SHADER_ID = "EnvironmentShader";

		// Descriptor Sets
		const std::string GEOMETRY_SHADER_DSL_ID = "GeometryShaderDSL";
		const std::string GEOMETRY_SHADER_DS_ID = "GeometryShaderDS";

		const std::string BINDLESS_TEXTURE_DSL_ID = "BindlessTextureDSL";
		const std::string BINDLESS_TEXTURE_DS_ID = "BindlessTextureDS";

		const std::string ENVIRONMENT_SHADER_DSL_ID = "EnvironmentShaderDSL";
		const std::string ENVIRONMENT_SHADER_DS_ID = "EnvironmentShaderDS";

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