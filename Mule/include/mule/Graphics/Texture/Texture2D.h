#pragma once

// Engine
#include "WeakRef.h"
#include "ITexture.h"
#include "Asset/Asset.h"
#include "Graphics/RenderTypes.h"

// Submodules
#include <imgui.h>

// STD
#include <filesystem>
namespace fs = std::filesystem;

namespace Mule
{
	class GraphicsContext;

	class Texture2D : public ITexture
	{
	public:
		explicit Texture2D(WeakRef<GraphicsContext> context, void* data, int width, int height, int mips, TextureFormat format, TextureFlags flags);
		explicit Texture2D(WeakRef<GraphicsContext> context, const std::string& name, void* data, int width, int height, int mips, TextureFormat format, TextureFlags flags);
		explicit Texture2D(WeakRef<GraphicsContext> context, const fs::path& filepath, void* data, int width, int height, int mips, TextureFormat format, TextureFlags flags);
		virtual ~Texture2D();

		ImTextureID GetImGuiID() const { return mImGuiID; }
	private:
		ImTextureID mImGuiID;
	};
}