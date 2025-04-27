
#include "Graphics/API/Image.h"

#include <stb/stb_image.h>

namespace Mule
{
	Image::Image()
		:
		Width(0),
		Height(0),
		Data(),
		Format(TextureFormat::NONE)
	{
	}

	Image Image::Load(const std::filesystem::path& filepath)
	{
		Image image;
		int width, height, channels;

		void* data = stbi_load(filepath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (data == nullptr)
		{
			return Image();
		}

		// TODO: eventually all0ow this to load different formats and not force RGBA
		const uint32_t pizelSize = 4;

		image.Data = Buffer(data, width * height * pizelSize);
		image.Format = TextureFormat::RGBA_8U;
		image.Width = width;
		image.Height = height;

		return image;
	}
}