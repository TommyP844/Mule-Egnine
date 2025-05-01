#include "Graphics/API/ShaderResourceGroup.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/VulkanDescriptorSet.h"

namespace Mule
{
	Ref<ShaderResourceGroup> ShaderResourceGroup::Create(Ref<ShaderResourceBlueprint> blueprint)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanDescriptorSet>(blueprint);
		case Mule::GraphicsAPI::None:
		default:
			return nullptr;
		}
	}

	Ref<ShaderResourceGroup> ShaderResourceGroup::Create(const std::vector<ShaderResourceDescription>& blueprintDescriptions)
	{
		Ref<ShaderResourceBlueprint> blueprint = ShaderResourceBlueprint::Create(blueprintDescriptions);

		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanDescriptorSet>(blueprint);
		case Mule::GraphicsAPI::None:
		default:
			return nullptr;
		}
	}
}