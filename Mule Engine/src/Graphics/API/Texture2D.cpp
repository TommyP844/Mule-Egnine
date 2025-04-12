
#include "Graphics/API/Texture2D.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Texture/VulkanTexture2D.h"

namespace Mule
{
	Ref<Texture2D> Texture2D::Create(void* data, uint32_t width, uint32_t height, TextureFormat format, TextureFlags flags)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();
		switch (API)
		{
		case GraphicsAPI::None:

			break;
		case GraphicsAPI::Vulkan: return Ref<Vulkan::VulkanTexture2D>(new Vulkan::VulkanTexture2D(data, width, height, format, flags));
		}
	}
}