#include "Asset/Serializer/TextureSerializer.h"

#include "Graphics/API/Image.h"

#include <spdlog/spdlog.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Mule
{
	TextureSerializer::TextureSerializer(Ref<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	Ref<Texture2D> TextureSerializer::Load(const fs::path& filepath)
	{		
		Image image = Image::Load(filepath);

		auto texture = Texture2D::Create("", image.Data, image.Width, image.Height, image.Format, TextureFlags::GenerateMips | TextureFlags::TransferDst);
		image.Data.Release();

		return texture;
	}
	
	void TextureSerializer::Save(Ref<Texture2D> asset)
	{
	}
}