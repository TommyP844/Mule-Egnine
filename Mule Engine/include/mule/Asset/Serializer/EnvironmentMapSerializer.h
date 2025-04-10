#pragma once

#include "Ref.h"
#include "Asset/AssetManager.h"
#include "IAssetSerializer.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Shader/ComputeShader.h"

#include <mutex>
#include <future>

namespace Mule
{
	class EngineContext;

	class EnvironmentMapSerializer : public IAssetSerializer<EnvironmentMap, AssetType::EnvironmentMap>
	{
	public:
		EnvironmentMapSerializer(WeakRef<GraphicsContext> context, WeakRef<EngineContext> engineContext);

		virtual Ref<EnvironmentMap> Load(const fs::path& filepath) override;
		virtual void Save(Ref<EnvironmentMap> asset) override;

	private:
		std::mutex mMutex;

		AssetHandle mBRDFLutMap;
		WeakRef<GraphicsContext> mContext;
		WeakRef<EngineContext> mEngineContext;

		Ref<ComputeShader> mCubeMapCompute;
		Ref<ComputeShader> mDiffuseIBLCompute;
		Ref<ComputeShader> mPreFilterCompute;
		Ref<ComputeShader> mBRDFCompute;
		Ref<DescriptorSetLayout> mCubeMapDescriptorSetLayout;
		Ref<DescriptorSetLayout> mBRDFDescriptorSetLayout;
		Ref<DescriptorSet> mBRDFDescriptorSet;
		Ref<DescriptorSet> mCubeMapDescriptorSet;

		std::future<void> mShaderLoadFuture;
	};
}
