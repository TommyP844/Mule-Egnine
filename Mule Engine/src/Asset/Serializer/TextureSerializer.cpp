#include "Asset/Serializer/TextureSerializer.h"

#include <spdlog/spdlog.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Mule
{
	TextureSerializer::TextureSerializer(WeakRef<GraphicsContext> context)
		:
		mContext(context)
	{
	}

	Ref<Texture2D> TextureSerializer::Load(const fs::path& filepath)
	{
		TextureFormat format;
		int width, height, components;
		void* data;
		if (filepath.extension().string() == ".hdr")
		{
			data = stbi_loadf(filepath.string().c_str(), &width, &height, &components, STBI_rgb_alpha);
			format = TextureFormat::RGBA32F;
		}
		else
		{
			data = stbi_load(filepath.string().c_str(), &width, &height, &components, STBI_rgb_alpha);
			format = TextureFormat::RGBA8U;
		}

		if (data == nullptr || width < 1 || height < 1)
		{
			SPDLOG_ERROR("Failed to load texture: {}", filepath.string());
			return nullptr;
		}

		auto texture = MakeRef<Texture2D>(mContext, filepath, data, width, height, format, TextureFlags::GenerateMips);
		stbi_image_free(data);
		return texture;
	}
	
	void TextureSerializer::Save(Ref<Texture2D> asset)
	{
	}
}