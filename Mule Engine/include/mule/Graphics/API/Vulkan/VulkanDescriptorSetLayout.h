#pragma once

#include "WeakRef.h"
#include "VulkanRenderTypes.h"

// Submodules
#include <Volk/volk.h>

// STD
#include <vector>

namespace Mule
{
	class GraphicsContext;

	class VulkanDescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout(const std::vector<LayoutDescription>& layouts);
		~VulkanDescriptorSetLayout();

		VkDescriptorSetLayout GetLayout() const { return mLayout; }

	private:
		VkDevice mDevice;
		VkDescriptorSetLayout mLayout;
	};
}
