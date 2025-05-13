#include "Graphics/API/TimelineSemaphore.h"

#include "Graphics/API/GraphicsContext.h"

#include "Graphics/API/Vulkan/Syncronization/VulkanTimelineSemaphore.h"

namespace Mule
{
	Ref<TimelineSemaphore> TimelineSemaphore::Create()
	{
		GraphicsAPI api = GraphicsContext::Get().GetAPI();
		switch (api)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanTimelineSemaphore>();
		case Mule::GraphicsAPI::None:
		default:
			break;
		}
	}
}