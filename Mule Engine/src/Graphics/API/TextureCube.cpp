
#include "Graphics/API/TextureCube.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Texture/VulkanTextureCube.h"

namespace Mule
{
	Ref<TextureCube> TextureCube::Create(const std::string& name, const Buffer& buffer, uint32_t width, TextureFormat format, TextureFlags flags)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanTextureCube>(name, buffer, width, format, flags);
		case Mule::GraphicsAPI::None:
		default:
			return nullptr;
		}
	}

	TextureCube::TextureCube(const std::string& name, TextureFormat format, TextureFlags flags)
		:
		Texture(name, format, flags, TextureType::TextureType_Cube)
	{
	}
}