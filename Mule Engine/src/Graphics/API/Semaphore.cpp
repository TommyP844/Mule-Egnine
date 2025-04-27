
#include "Graphics/API/Semaphore.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Syncronization/VulkanSemaphore.h"

namespace Mule
{
	Ref<Semaphore> Semaphore::Create()
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case GraphicsAPI::None: return nullptr;
		case GraphicsAPI::Vulkan: return Ref<Vulkan::VulkanSemaphore>(new Vulkan::VulkanSemaphore());
		}

		return nullptr;
	}
}