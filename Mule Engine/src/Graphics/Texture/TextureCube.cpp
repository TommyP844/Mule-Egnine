
#include "Graphics/Texture/TextureCube.h"

namespace Mule
{
	TextureCube::TextureCube(WeakRef<GraphicsContext> context, void* data, uint32_t axisSize, uint32_t layers, TextureFormat format, TextureFlags flags)
		:
		ITexture(context)
	{
		Initialize(data, axisSize, axisSize, 1, layers, format, TextureFlags(TextureFlags::CubeMap | flags));
	}
}