#pragma once

#include "Asset/AssetType.h"
#include "Ref.h"

#include <future>
#include <filesystem>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace Mule
{
	class IBaseLoader
	{
	public:
		virtual ~IBaseLoader() {}
	};



	template<typename T, AssetType Type>
	class IAssetLoader : public IBaseLoader
	{
	public:
		virtual Ref<T> Load(const fs::path& filepath) = 0;

		static constexpr AssetType sType = Type;
	};
}