#include "Graphics/DescriptorSetLayout.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	DescriptorSetLayout::DescriptorSetLayout(WeakRef<GraphicsContext> context, const DescriptorSetLayoutDescription& description)
		:
		mDevice(context->GetDevice()),
		mLayout(VK_NULL_HANDLE)
	{
		std::vector<VkDescriptorSetLayoutBinding> layouts;
		for (const auto& layout : description.Layouts)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = layout.Binding;
			layoutBinding.descriptorCount = layout.ArrayCount;
			layoutBinding.descriptorType = (VkDescriptorType)layout.Type;
			layoutBinding.stageFlags = (VkShaderStageFlags)layout.Stage;
			layoutBinding.pImmutableSamplers = nullptr;
			layouts.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.bindingCount = layouts.size();
		createInfo.pBindings = layouts.data();

		VkResult result = vkCreateDescriptorSetLayout(mDevice, &createInfo, nullptr, &mLayout);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create descriptor set layout");
			return;
		}
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(mDevice, mLayout, nullptr);
	}
}