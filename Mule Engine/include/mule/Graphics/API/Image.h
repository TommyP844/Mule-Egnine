#pragma once

#include "Buffer.h"
#include "GraphicsCore.h"

#include <filesystem>

namespace Mule
{
	class Image
	{
	public:
		Image();
		virtual ~Image() = default;

		uint32_t Width;
		uint32_t Height;
		Buffer Data;
		TextureFormat Format;

		static Image Load(const std::filesystem::path& filepath);
	};
}