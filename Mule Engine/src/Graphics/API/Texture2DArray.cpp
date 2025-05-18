#include "Graphics/API/Texture2DArray.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Texture/VulkanTexture2DArray.h"

namespace Mule
{
	Ref<Texture2DArray> Texture2DArray::Create(const std::string& name, const Buffer& data, uint32_t width, uint32_t height, uint32_t layers, TextureFormat format, TextureFlags flags)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();
		switch (API)
		{
		case GraphicsAPI::Vulkan: return Ref<Vulkan::VulkanTexture2DArray>(new Vulkan::VulkanTexture2DArray(name, data, width, height, layers, format, flags));
		case GraphicsAPI::None:
			assert(false && "Invalid API");
			break;
		}
	}

	Texture2DArray::Texture2DArray(const std::string& name, TextureFormat format, TextureFlags flags)
		:
		Texture(name, format, flags, TextureType::TextureType_2D)
	{
	}
}