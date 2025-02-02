#pragma once

#include "Ref.h"
#include "Asset/AssetManager.h"
#include "AssetLoader.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/ComputeShader.h"

namespace Mule
{
	class EnvironmentMapLoader : public IAssetLoader<EnvironmentMap, AssetType::EnvironmentMap>
	{
	public:
		EnvironmentMapLoader(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager);

		Ref<EnvironmentMap> LoadText(const fs::path& filepath) override;
		void SaveText(Ref<EnvironmentMap> asset) override;
		Ref<EnvironmentMap> LoadBinary(const Buffer& buffer) override;
		void SaveBinary(Ref<EnvironmentMap> asset) override;

	private:
		WeakRef<GraphicsContext> mContext;
		WeakRef<AssetManager> mAssetManager;

		Ref<ComputeShader> mCubeMapCompute;
		Ref<DescriptorSetLayout> mDescriptorSetLayout;
	};
}
