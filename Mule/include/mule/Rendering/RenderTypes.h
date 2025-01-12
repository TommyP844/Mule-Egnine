#pragma once

#include <bgfx/bgfx.h>

namespace Mule
{
	enum class TextureFormat : uint32_t
	{
		RGB8 = bgfx::TextureFormat::RGB8,
		RGBA8 = bgfx::TextureFormat::RGBA8,
		D32F = bgfx::TextureFormat::D32F
	};
}