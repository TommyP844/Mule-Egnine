#pragma once

// Engine
#include "Asset/Asset.h"

// Submodules
#include <bgfx/bgfx.h>
#include <imgui.h>

// STD
#include <filesystem>
namespace fs = std::filesystem;

namespace Mule
{
	class Texture2D : public Asset<AssetType::Texture>
	{
	public:
		Texture2D(void* data, int width, int height, bgfx::TextureFormat::Enum format, bool createMips);
		Texture2D(const fs::path& filepath);

		bgfx::TextureHandle GetRenderHandle() const { return mHandle; }
		ImTextureID GetImGuiID() const { return (ImTextureID)mHandle.idx; }
	private:

		bgfx::TextureHandle mHandle = bgfx::TextureHandle();

	};
}