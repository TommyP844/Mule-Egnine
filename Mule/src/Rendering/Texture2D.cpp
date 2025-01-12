#include "Rendering/Texture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <spdlog/spdlog.h>

namespace Mule
{
	Texture2D::Texture2D(void* data, int width, int height, bool createMips)
		:
		Asset()
	{

	}

	Texture2D::Texture2D(const fs::path& filepath)
		:
		Asset(filepath)
	{
		int width = -1, height = -1, comp;
		std::string file = filepath.string();
		void* data = stbi_load(file.c_str(), &width, &height, &comp, STBI_rgb_alpha);
		if (width < 1 || height < 1 || comp < 4 || data == nullptr)
		{
			SPDLOG_ERROR("Failed to load texture: {}", filepath.string());
			SPDLOG_ERROR("\tError: {}", stbi_failure_reason());
			return;
		}
	}

	Texture2D::~Texture2D()
	{
	}
}