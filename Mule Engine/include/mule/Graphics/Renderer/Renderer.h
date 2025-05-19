#pragma once

#include "Ref.h"

#include "Graphics/Renderer/RenderGraph/RenderGraph.h"
#include "Graphics/Renderer/RenderGraph/ResourceBuilder.h"
#include "Graphics/Renderer/CommandList.h"
#include "Graphics/Camera.h"
#include "Graphics/GuidArray.h"
#include "Graphics/GPUObjects.h"

#include <vector>
#include <mutex>

namespace Mule
{
	class Renderer
	{
	public:
		~Renderer() = default;

		static void Init();
		static void Shutdown();
		static Renderer& Get();

		// TODO: we need to keep a weak reference to all resource registrys so we can properly wait on them when updating global resources like bindless textures
		Ref<ResourceRegistry> CreateResourceRegistry();
		void Submit(const Camera& camera, const CommandList& commandList);

		void SetQuadMesh(WeakRef<Mesh> quadMesh) { mQuadMesh = quadMesh; }
		void SetDefaultCubeMap(WeakRef<TextureCube> cubeMap) { mDefaultCubeMap = cubeMap; }
		void SetDefaultTexture(WeakRef<Texture2D> texture) { mDefaultTexture = texture; }

		void Render();

		void AddTexture(WeakRef<Texture> texture);
		void RemoveTexture(WeakRef<Texture> texture);

		void AddMaterial(WeakRef<Material> material);
		void UpdateMaterial(WeakRef<Material> material);
		void RemoveMaterial(WeakRef<Material> material);

		uint32_t GetFramesInFlight() const { return mFramesInFlight; }
		uint32_t GetFrameIndex() const { return mFrameIndex; }

	private:
		Renderer();
		void BuildGraph();
		void UpdateBindlessResources();
		bool BindlessResourcesNeedUpdate();

		static Renderer* sRenderer;

		struct RenderRequest
		{
			Camera Camera;
			CommandList Commands;
		};

		std::mutex mMutex;
		std::mutex mResourceMutex;
		std::vector<RenderRequest> mRenderRequests;
		Ref<RenderGraph> mRenderGraph;
		uint32_t mFramesInFlight;
		uint32_t mFrameIndex;

		ResourceBuilder mResourceBuilder;

		ResourceHandle mBindlessTextureSRGHandle;
		ResourceHandle mBindlessMaterialBufferHandle;
		ResourceHandle mBindlessMaterialSRGHandle;

		std::vector<Ref<ShaderResourceGroup>> mBindlessTextureSRG;
		std::vector<Ref<ShaderResourceGroup>> mBindlessMaterialSRG;
		std::vector<Ref<UniformBuffer>> mBindlessMaterialBuffer;

		Ref<Texture2D> mBlackTex;
		Ref<Texture2D> mWhiteTex;
		Ref<Texture2D> mDefaultNormalTex;
		Ref<Material> mDefaultMaterial;

		WeakRef<Mesh> mQuadMesh;
		WeakRef<TextureCube> mDefaultCubeMap;
		WeakRef<Texture2D> mDefaultTexture;

		// Bindless Resources
		GuidArray<WeakRef<Texture>> mBindlessTextureIndices;
		GuidArray<GPU::Material> mBindlessMaterialIndices;

		struct BindlessResourceUpdate
		{
			std::vector<WeakRef<Texture>> AddTextures;
			std::vector<WeakRef<Texture>> RemoveTextures;

			std::vector<WeakRef<Material>> AddMaterials;
			std::vector<WeakRef<Material>> UpdateMaterials;
			std::vector<WeakRef<Material>> RemoveMaterials;
		};

		std::vector<BindlessResourceUpdate> mResourceUpdates;
	};
}