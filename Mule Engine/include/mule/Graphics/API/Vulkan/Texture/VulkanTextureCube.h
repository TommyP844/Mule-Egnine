#pragma once

#include "IVulkanTexture.h"

namespace Mule
{
	class VulkanTextureCube : public IVulkanTexture
	{
	public:
		VulkanTextureCube(void* data, uint32_t axisSize, uint32_t layers, TextureFormat format, TextureFlags flage = TextureFlags::None);
	};
}