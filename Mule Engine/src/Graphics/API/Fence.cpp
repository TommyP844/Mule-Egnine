
#include "Graphics/API/Fence.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Syncronization/VulkanFence.h"

namespace Mule
{
	Ref<Fence> Fence::Create()
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case GraphicsAPI::None: return nullptr;
		case GraphicsAPI::Vulkan: return Ref<Vulkan::VulkanFence>(new Vulkan::VulkanFence());
		}

		return nullptr;
	}
}