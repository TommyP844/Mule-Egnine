#pragma once

#include "WeakRef.h"
#include "RenderTypes.h"

// Submodules
#include <Volk/volk.h>

// STD
#include <vector>

namespace Mule
{
	class GraphicsContext;

	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(WeakRef<GraphicsContext> context, const std::vector<LayoutDescription>& layouts);
		~DescriptorSetLayout();

		VkDescriptorSetLayout GetLayout() const { return mLayout; }

	private:
		VkDevice mDevice;
		VkDescriptorSetLayout mLayout;
	};
}
