
#include "Graphics/API/ShaderResourceBlueprint.h"
#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"

namespace Mule
{
	Ref<ShaderResourceBlueprint> ShaderResourceBlueprint::Create(const std::initializer_list<ShaderResourceDescription>& resources)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanDescriptorSetLayout>(resources);
		case GraphicsAPI::None:
		default:
			return nullptr;
		}
	}
}