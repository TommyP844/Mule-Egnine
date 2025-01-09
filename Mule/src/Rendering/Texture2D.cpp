#include "Rendering/Texture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <spdlog/spdlog.h>

namespace Mule
{
	Texture2D::Texture2D(void* data, int width, int height, bgfx::TextureFormat::Enum format, bool createMips)
		:
		Asset()
	{
		uint64_t flags = 0;
		if (createMips) 
		{
			flags |= BGFX_RESOLVE_AUTO_GEN_MIPS;
		}

		bgfx::TextureInfo info;
		bgfx::calcTextureSize(info, width, height, 1, false, false, 1, format);
		uint32_t size = info.storageSize;

		const bgfx::Memory* mem = bgfx::copy(data, size);
		
		mHandle = bgfx::createTexture2D(
			(uint16_t)width,
			(uint16_t)height,
			false,
			1,
			format,
			flags,
			mem);
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

		bgfx::TextureInfo info;
		bgfx::calcTextureSize(info, width, height, 1, false, false, 1, bgfx::TextureFormat::RGBA8);
		uint32_t size = info.storageSize;

		const bgfx::Memory* mem = bgfx::copy(data, size);

		mHandle = bgfx::createTexture2D(
			(uint16_t)width,
			(uint16_t)height,
			false,
			1,
			bgfx::TextureFormat::RGBA8,
			BGFX_SAMPLER_BITS_MASK,
			mem);
	}
}