#pragma once

#include "ITexture.h"

namespace Mule
{
	class TextureCube : public ITexture
	{
	public:
		TextureCube(WeakRef<GraphicsContext> context, void* data, uint32_t axisSize, uint32_t layers, TextureFormat format, TextureFlags flage = TextureFlags::None);
	};
}