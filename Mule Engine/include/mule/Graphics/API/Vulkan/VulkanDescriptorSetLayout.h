#pragma once

#include "Graphics/API/ShaderResourceBlueprint.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanDescriptorSetLayout : public ShaderResourceBlueprint
	{
	public:
		VulkanDescriptorSetLayout(const std::vector<ShaderResourceDescription>& resources);
		~VulkanDescriptorSetLayout();

		VkDescriptorSetLayout GetLayout() const { return mLayout; }

	private:
		VkDescriptorSetLayout mLayout;
	};
}
