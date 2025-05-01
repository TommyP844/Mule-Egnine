#pragma once

#include "Graphics/API/Texture2D.h"

#include "IVulkanTexture.h"
#include "Asset/Asset.h"
#include "VulkanImage.h"
#include "Buffer.h"

// Submodules
#include <imgui.h>

// STD
#include <filesystem>
namespace fs = std::filesystem;

namespace Mule::Vulkan
{
	class VulkanTexture2D : public Texture2D, public IVulkanTexture
	{
	public:
		explicit VulkanTexture2D(const std::string& name, const Buffer&, int width, int height, TextureFormat format, TextureFlags flags);
		virtual ~VulkanTexture2D();

		uint32_t GetWidth() override;
		uint32_t GetHeight() override;
		uint32_t GetDepth() override;
		uint32_t GetMipLevels() override;
		uint32_t GetArrayLayers() override;

		ImTextureID GetImGuiID(uint32_t mipLevel = 0, uint32_t arrayLayer = 0) const override;
		WeakRef<TextureView> GetView(uint32_t mipLevel, uint32_t arrayLayer = 0) const override;
		WeakRef<TextureView> GetMipView(uint32_t mipLevel) override;
		void TransitionImageLayoutImmediate(ImageLayout newLayout) override;
		Buffer ReadTextureData(uint32_t mipLevel = 0) override;
		void WriteMipLevel(uint32_t mipLevel, const Buffer& data) override;
	private:
		bool mHasMips;


	};
}