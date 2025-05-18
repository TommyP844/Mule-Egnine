
#include "Graphics/API/Texture2D.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Texture/VulkanTexture2D.h"

namespace Mule
{
	Ref<Texture2D> Texture2D::Create(const std::string& name, const Buffer& data, uint32_t width, uint32_t height, TextureFormat format, TextureFlags flags)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();
		switch (API)
		{
		case GraphicsAPI::Vulkan: return Ref<Vulkan::VulkanTexture2D>(new Vulkan::VulkanTexture2D(name, data, width, height, format, flags));
		case GraphicsAPI::None:
			assert(false && "Invalid API");
			break;
		}
	}

	Texture2D::Texture2D(const std::string& name, TextureFormat format, TextureFlags flags)
		:
		Texture(name, format, flags, TextureType::TextureType_2D)
	{
	}
}