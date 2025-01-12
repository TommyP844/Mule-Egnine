#pragma once

// Engine
#include "Asset/Asset.h"

// Submodules
#include <imgui.h>

// STD
#include <filesystem>
namespace fs = std::filesystem;

namespace Mule
{
	class Texture2D : public Asset<AssetType::Texture>
	{
	public:
		Texture2D() {}
		Texture2D(void* data, int width, int height, bool createMips);
		Texture2D(const fs::path& filepath);
		~Texture2D();

		ImTextureID GetImGuiID() const { return (ImTextureID)0; }
	private:


	};
}