#include "Graphics/API/CommandAllocator.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Execution/VulkanCommandPool.h"

namespace Mule

{
	Ref<CommandAllocator> CommandAllocator::Create()
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::None: return nullptr;
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanCommandPool>();
		}

		return nullptr;
	}
}