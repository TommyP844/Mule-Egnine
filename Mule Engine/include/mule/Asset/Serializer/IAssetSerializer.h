#pragma once

#include "Asset/AssetType.h"
#include "Ref.h"
#include "Services/ServiceManager.h"

#include <filesystem>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace Mule
{
	class EngineContext;

	class IBaseSerializer
	{
	public:
		virtual ~IBaseSerializer() {}
	};

	template<typename T, AssetType Type>
	class IAssetSerializer : public IBaseSerializer
	{
	public:
		IAssetSerializer(Ref<ServiceManager> serviceManager)
			:
			mServiceManager(serviceManager)
		{ }

		virtual Ref<T> Load(const fs::path& filepath) = 0;
		virtual void Save(Ref<T> asset) = 0;

		static constexpr AssetType sType = Type;

	protected:
		Ref<ServiceManager> mServiceManager;
	};
}