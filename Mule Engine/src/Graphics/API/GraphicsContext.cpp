#include "Graphics/API/GraphicsContext.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	GraphicsContext* GraphicsContext::mInstance = nullptr;

	GraphicsContext::GraphicsContext(GraphicsAPI api, Ref<Window> window)
		:
		mAPI(api)
	{
		switch (api)
		{
		case GraphicsAPI::None:

			break;
		case GraphicsAPI::Vulkan:
			Vulkan::VulkanContext::Init(window);
			break;
		}
	}

	void GraphicsContext::Init(GraphicsAPI api, Ref<Window> window)
	{
		mInstance = new GraphicsContext(api, window);
	}

	void GraphicsContext::Shutdown()
	{
		switch (mInstance->mAPI)
		{
		case GraphicsAPI::Vulkan: Vulkan::VulkanContext::Shutdown(); break;
		default:
			SPDLOG_ERROR("Attempting to shutdown invalid graphics API");
			break;
		}
		delete mInstance;
	}

	GraphicsContext& GraphicsContext::Get()
	{
		assert(mInstance != nullptr && "Graphics Context not initialized");
		return *mInstance;
	}

	bool GraphicsContext::NewFrame()
	{
		switch (mAPI)
		{
		case GraphicsAPI::Vulkan: return Vulkan::VulkanContext::Get().BeginFrame();
		case GraphicsAPI::None:
		default:
			return false;
		}
	}

	void GraphicsContext::EndFrame(const std::vector<Ref<Semaphore>>& waitSemaphores)
	{
		switch (mAPI)
		{
		case GraphicsAPI::None:

			break;

		case GraphicsAPI::Vulkan:
		{
			std::vector<VkSemaphore> semaphores(waitSemaphores.size());
			for (uint32_t i = 0; i < waitSemaphores.size(); i++)
			{
				Ref<Vulkan::VulkanSemaphore> vkSemaphore = waitSemaphores[i];
				semaphores[i] = vkSemaphore->GetHandle();
			}
			Vulkan::VulkanContext::Get().EndFrame(semaphores);
		}
			break;

		}
	}

	void GraphicsContext::ResizeSwapchain(uint32_t width, uint32_t height)
	{
		switch (mAPI)
		{
		case Mule::GraphicsAPI::Vulkan:
			Vulkan::VulkanContext::Get().ResizeSwapchain(width, height);
			break;
		case Mule::GraphicsAPI::None:
		default:
			break;
		}
	}

	void GraphicsContext::AwaitIdle()
	{
		switch (mAPI)
		{
		case Mule::GraphicsAPI::Vulkan:
			Vulkan::VulkanContext::Get().AwaitIdle();
			break;
		case Mule::GraphicsAPI::None:
		default:
			break;
		}
	}
}