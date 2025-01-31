#pragma once

#include "WeakRef.h"
#include "RenderTypes.h"

// Submodules
#include <vulkan/vulkan.h>

// STD
#include <vector>

namespace Mule
{
	class GraphicsContext;

	struct DescriptorSetLayoutDescription
	{
		std::vector<LayoutDescription> Layouts;
	};

	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(WeakRef<GraphicsContext> context, const DescriptorSetLayoutDescription& description);
		~DescriptorSetLayout();

		VkDescriptorSetLayout GetLayout() const { return mLayout; }

	private:
		VkDevice mDevice;
		VkDescriptorSetLayout mLayout;

	};
}
