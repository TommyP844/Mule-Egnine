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

	class Texture2D : public Asset<AssetType::Texture>, public ITexture
	{
	public:
		Texture2D(WeakRef<GraphicsContext> context, void* data, int width, int height, TextureFormat format, TextureFlags flags);
		virtual ~Texture2D();

		ImTextureID GetImGuiID() const { return (ImTextureID)0; }
	private:

	};
}