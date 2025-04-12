#pragma once

#include "Graphics/API/Texture2D.h"

#include "IVulkanTexture.h"
#include "Asset/Asset.h"
#include "VulkanImage.h"

// Submodules
#include <imgui.h>

// STD
#include <filesystem>
namespace fs = std::filesystem;

namespace Mule::Vulkan
{
	class GraphicsContext;

	class VulkanTexture2D : public IVulkanTexture, public Texture2D
	{
	public:
		explicit VulkanTexture2D(void* data, int width, int height, TextureFormat format, TextureFlags flags);
		virtual ~VulkanTexture2D();

		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;
		TextureFormat GetFormat() const override;
		TextureFlags GetFlags() const override;

		ImTextureID GetImGuiID() const { return mImGuiID; }
	private:
		ImTextureID mImGuiID;
		VulkanImage mImage;
	};
}