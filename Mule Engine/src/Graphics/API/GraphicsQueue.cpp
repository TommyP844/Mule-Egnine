#include "Graphics/API/GraphicsQueue.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Execution/VulkanQueue.h"


namespace Mule
{
	Ref<GraphicsQueue> GraphicsQueue::Create()
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case GraphicsAPI::None: return nullptr;
		case GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanQueue>();
		}

		return nullptr;
	}
}