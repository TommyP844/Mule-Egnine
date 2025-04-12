#include "Graphics/API/Vulkan/Texture/VulkanTexture2D.h"

#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	VulkanTexture2D::VulkanTexture2D(void* data, int width, int height, TextureFormat format, TextureFlags flags)
		:
		Texture2D()
	{
		
	}

	VulkanTexture2D::~VulkanTexture2D()
	{

	}

	uint32_t VulkanTexture2D::GetWidth() const
	{
		return 0;
	}

	uint32_t VulkanTexture2D::GetHeight() const
	{
		return 0;
	}

	TextureFormat VulkanTexture2D::GetFormat() const
	{
		return TextureFormat();
	}

	TextureFlags VulkanTexture2D::GetFlags() const
	{
		return TextureFlags();
	}
}