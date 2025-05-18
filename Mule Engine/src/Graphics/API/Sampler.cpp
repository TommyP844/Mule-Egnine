#include "Graphics/API/Sampler.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Texture/VulkanSampler.h"

namespace Mule
{
	Ref<Sampler> Sampler::Create(const SamplerDescription& description)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanSampler>(description);
		case GraphicsAPI::None:
		default:
			return nullptr;
		}
	}
}