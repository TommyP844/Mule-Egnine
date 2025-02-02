
#include "Graphics/Texture/TextureCube.h"

namespace Mule
{
	TextureCube::TextureCube(WeakRef<GraphicsContext> context, void* data, uint32_t axisSize, uint32_t layers, TextureFormat format)
		:
		ITexture(context)
	{
		Initialize(data, axisSize, axisSize, 1, layers, 1, format, TextureFlags::CubeMap);
	}
}