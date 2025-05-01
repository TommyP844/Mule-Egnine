#pragma once

#include "Ref.h"
#include "Asset/AssetManager.h"
#include "IAssetSerializer.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/API/ComputePipeline.h"
#include "Graphics/API/ShaderResourceGroup.h"
#include "Graphics/API/ShaderResourceBlueprint.h"

#include <mutex>
#include <future>

namespace Mule
{
	class EngineContext;

	class EnvironmentMapSerializer : public IAssetSerializer<EnvironmentMap, AssetType::EnvironmentMap>
	{
	public:
		EnvironmentMapSerializer(Ref<ServiceManager> serviceManager);

		Ref<EnvironmentMap> Load(const fs::path& filepath) override;
		void Save(Ref<EnvironmentMap> asset) override;

	private:
		std::mutex mMutex;

		AssetHandle mBRDFLutMap;

		Ref<ComputePipeline> mCubeMapCompute;
		Ref<ComputePipeline> mDiffuseIBLCompute;
		Ref<ComputePipeline> mPreFilterCompute;
		Ref<ComputePipeline> mBRDFCompute;
		Ref<ShaderResourceBlueprint> mCubeMapDescriptorSetLayout;
		Ref<ShaderResourceBlueprint> mBRDFDescriptorSetLayout;
		Ref<ShaderResourceGroup> mBRDFDescriptorSet;
		Ref<ShaderResourceGroup> mCubeMapDescriptorSet;

		std::future<void> mShaderLoadFuture;
	};
}
