#pragma once

#include "Asset/AssetType.h"
#include "Ref.h"
#include "Buffer.h"

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
		virtual Ref<T> LoadText(const fs::path& filepath) = 0;
		virtual void SaveText(Ref<T> asset) = 0;

		virtual Ref<T> LoadBinary(const Buffer& buffer) = 0;
		virtual void SaveBinary(Ref<T> asset) = 0;

		static constexpr AssetType sType = Type;
	};
}