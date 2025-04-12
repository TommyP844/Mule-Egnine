#include "Graphics/API/GraphicsContext.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

namespace Mule
{
	GraphicsContext* GraphicsContext::mInstance = nullptr;

	GraphicsContext::GraphicsContext(GraphicsAPI api, WeakRef<Window> window)
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

	void GraphicsContext::Init(GraphicsAPI api, WeakRef<Window> window)
	{
		mInstance = new GraphicsContext(api, window);
	}

	void GraphicsContext::Shutdown()
	{
		delete mInstance;
	}

	GraphicsContext& GraphicsContext::Get()
	{
		assert(mInstance != nullptr && "Graphics Context not initialized");
		return *mInstance;
	}

	void GraphicsContext::NewFrame()
	{
		switch (mAPI)
		{
		case GraphicsAPI::None:

			break;

		case GraphicsAPI::Vulkan:
			Vulkan::VulkanContext::Get().BeginFrame();
			break;

		}
	}

	void GraphicsContext::EndFrame()
	{
		switch (mAPI)
		{
		case GraphicsAPI::None:

			break;

		case GraphicsAPI::Vulkan:
			Vulkan::VulkanContext::Get().EndFrame({});
			break;

		}
	}
}