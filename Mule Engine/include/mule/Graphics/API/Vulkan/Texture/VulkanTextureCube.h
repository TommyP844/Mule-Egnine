#pragma once

#include "Graphics/API/TextureCube.h"
#include "Graphics/API/Vulkan/Texture/IVulkanTexture.h"

namespace Mule::Vulkan
{
	class VulkanTextureCube : public TextureCube, IVulkanTexture
	{
	public:
		VulkanTextureCube(const std::string& name, const Buffer& buffer, uint32_t width, TextureFormat format, TextureFlags flags);

		uint32_t GetWidth() override;
		uint32_t GetHeight() override;
		uint32_t GetDepth() override;
		uint32_t GetMipLevels() override;
		uint32_t GetArrayLayers() override;
		ImTextureID GetImGuiID(uint32_t mipLevel = 0, uint32_t arrayLayer = 0) const override;
		WeakRef<TextureView> GetView(uint32_t mipLevel, uint32_t arrayLayer = 0) const override;
	};
}