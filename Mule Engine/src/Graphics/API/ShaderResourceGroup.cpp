#include "Graphics/API/ShaderResourceGroup.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/VulkanDescriptorSet.h"

namespace Mule
{
	Ref<ShaderResourceGroup> ShaderResourceGroup::Create(const std::vector<Ref<ShaderResourceBlueprint>>& blueprints)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanDescriptorSet>(blueprints);
		case Mule::GraphicsAPI::None:
		default:
			return nullptr;
		}
	}
}