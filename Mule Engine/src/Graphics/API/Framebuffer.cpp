
#include "Graphics/API/Framebuffer.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/VulkanFrameBuffer.h"

namespace Mule
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferDescription& description)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanFramebuffer>(description);
		case Mule::GraphicsAPI::None:
		default:
			return nullptr;
		}
	}
}