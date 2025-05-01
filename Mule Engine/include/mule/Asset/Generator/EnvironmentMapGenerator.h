#pragma once

#include "Ref.h"

#include "Asset/AssetHandle.h"
#include "Asset/Generator/IAssetGenerator.h"
#include "Services/ServiceManager.h"

#include "Graphics/EnvironmentMap.h"
#include "Graphics/API/CommandAllocator.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/API/GraphicsQueue.h"
#include "Graphics/API/Fence.h"

#include <filesystem>
#include <vector>

namespace Mule
{
	class EnvironmentMapGenerator : public IAssetGenerator
	{
	public:
		EnvironmentMapGenerator(Ref<ServiceManager> serviceManager);
		virtual ~EnvironmentMapGenerator() = default;

		Ref<EnvironmentMap> Generate(const std::filesystem::path& outFilepath, AssetHandle hdrImageHandle, uint32_t size);
		Ref<EnvironmentMap> Generate(const std::filesystem::path& outFilepath, const std::vector<AssetHandle>& axisImages, uint32_t size);
	};
}
