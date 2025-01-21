#include "Graphics/Texture/Texture2D.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Mule
{
	Texture2D::Texture2D(WeakRef<GraphicsContext> context, void* data, int width, int height, TextureFormat format, TextureFlags flags)
		:
		Asset(),
		ITexture(context->GetDevice())
	{

	}

	Texture2D::~Texture2D()
	{
	}
}