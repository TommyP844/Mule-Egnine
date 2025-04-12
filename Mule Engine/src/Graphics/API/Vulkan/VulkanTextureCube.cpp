
#include "Graphics/API/Vulkan/VulkanTextureCube.h"

namespace Mule
{
	VulkanTextureCube::VulkanTextureCube(WeakRef<GraphicsContext> context, void* data, uint32_t axisSize, uint32_t layers, TextureFormat format, TextureFlags flags)
		:
		IVulkanTexture(context)
	{
		Initialize(data, axisSize, axisSize, 1, layers, format, TextureFlags(TextureFlags::CubeMap | flags));
	}
}