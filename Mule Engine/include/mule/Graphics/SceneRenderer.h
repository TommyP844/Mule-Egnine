#pragma once

#include "RenderGraph/RenderGraph.h"

#include "Services/ServiceManager.h"

#include "GuidArray.h"
#include "Graphics/Material.h"
#include "Graphics/API/Texture.h"
#include "Graphics/GPUObjects.h"

// STD
#include <vector>
#include <mutex>

#ifdef min
#undef min
#endif

namespace Mule
{
	class SceneRenderer : public RenderGraph::RenderGraph
	{
	public:
		SceneRenderer(Ref<ServiceManager> serviceManager);
		virtual ~SceneRenderer();
		
		Ref<Framebuffer> GetCurrentFrameBuffer() const override;

		uint32_t AddTexture(WeakRef<Texture> texture);
		uint32_t AddMaterial(WeakRef<Material> material);

		void UpdateMaterial(WeakRef<Material> material);

		void RemoveTexture(WeakRef<Texture> texture);
		void RemoveMaterial(WeakRef<Material> material);

	private:
		Ref<ServiceManager> mServiceManager;

		GuidArray<WeakRef<Texture>> mBindlessTextureArray;
		GuidArray<GPU::GPUMaterial> mBindlessMaterialArray;

		// Resources
		Mule::RenderGraph::ResourceHandle<Framebuffer> mMainFramebufferHandle;
		Mule::RenderGraph::ResourceHandle<ShaderResourceGroup> mBindlessTextureShaderResourceHandle;
		Mule::RenderGraph::ResourceHandle<UniformBuffer> mCameraBufferHandle;
		Mule::RenderGraph::ResourceHandle<UniformBuffer> mLightBufferHandle;
		Mule::RenderGraph::ResourceHandle<UniformBuffer> mMaterialBufferHandle;

		void PreRenderCallback(Ref<CommandBuffer> commandBuffer, WeakRef<Scene> scene, WeakRef<Camera> cameraOverride);
		void PostRenderCallback(Ref<CommandBuffer> commandBuffer, WeakRef<Scene> scene);
		void ResizeCallback(uint32_t width, uint32_t height);

		// Bindless Materials
		uint32_t QueryOrInsertTexture(AssetHandle handle, uint32_t defaultIndex = 0);
	};
}