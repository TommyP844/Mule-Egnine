
#include "Graphics/API/Vulkan/Texture/VulkanTextureCube.h"

#include "Graphics/API/Vulkan/VulkanTypeConversion.h"

namespace Mule::Vulkan
{
	VulkanTextureCube::VulkanTextureCube(const std::string& name, const Buffer& buffer, uint32_t width, TextureFormat format, TextureFlags flags)
		:
		TextureCube(name, format, flags)
	{
		InitCube(
			VK_IMAGE_TYPE_2D,
			GetVulkanFormat(format),
			width,
			width,
			1,
			1,
			6,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_VIEW_TYPE_CUBE
			);
	}

	uint32_t VulkanTextureCube::GetWidth()
	{
		return mWidth;
	}

	uint32_t VulkanTextureCube::GetHeight()
	{
		return mWidth;
	}

	uint32_t VulkanTextureCube::GetDepth()
	{
		return 1;
	}

	uint32_t VulkanTextureCube::GetMipLevels()
	{
		return mMipLevels;
	}

	uint32_t VulkanTextureCube::GetArrayLayers()
	{
		return mArrayLayers;
	}

	ImTextureID VulkanTextureCube::GetImGuiID(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return ImTextureID();
	}

	WeakRef<TextureView> VulkanTextureCube::GetView(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return WeakRef<TextureView>();
	}
}