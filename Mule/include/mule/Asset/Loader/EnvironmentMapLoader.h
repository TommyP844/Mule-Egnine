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

		virtual Ref<EnvironmentMap> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<EnvironmentMap> asset) override;
		virtual Ref<EnvironmentMap> LoadBinary(const Buffer& buffer) override;
		virtual void SaveBinary(Ref<EnvironmentMap> asset) override;

	private:
		AssetHandle mBRDFLutMap;
		WeakRef<GraphicsContext> mContext;
		WeakRef<AssetManager> mAssetManager;

		Ref<ComputeShader> mCubeMapCompute;
		Ref<ComputeShader> mDiffuseIBLCompute;
		Ref<ComputeShader> mPreFilterCompute;
		Ref<ComputeShader> mBRDFCompute;
		Ref<DescriptorSetLayout> mCubeMapDescriptorSetLayout;
		Ref<DescriptorSetLayout> mBRDFDescriptorSetLayout;
		Ref<DescriptorSet> mBRDFDescriptorSet;
		Ref<DescriptorSet> mCubeMapDescriptorSet;


	};
}
