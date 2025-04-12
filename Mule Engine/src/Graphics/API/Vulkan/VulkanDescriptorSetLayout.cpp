#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	VulkanDescriptorSetLayout::DescriptorSetLayout(WeakRef<VulkanContext> context, const std::vector<LayoutDescription>& layouts)
		:
		mDevice(context->GetDevice()),
		mLayout(VK_NULL_HANDLE)
	{
		std::vector<VkDescriptorSetLayoutBinding> vkLayouts;
		for (const auto& layout : layouts)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = layout.Binding;
			layoutBinding.descriptorCount = layout.ArrayCount;
			layoutBinding.descriptorType = (VkDescriptorType)layout.Type;
			layoutBinding.stageFlags = (VkShaderStageFlags)layout.Stage;
			layoutBinding.pImmutableSamplers = nullptr;
			vkLayouts.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.bindingCount = vkLayouts.size();
		createInfo.pBindings = vkLayouts.data();

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