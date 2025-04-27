#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/VulkanTypeConversion.h"

#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const std::vector<ShaderResourceDescription>& resources)
		:
		mLayout(VK_NULL_HANDLE)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		std::vector<VkDescriptorSetLayoutBinding> vkLayouts;
		for (const auto& resource : resources)
		{
			VkDescriptorSetLayoutBinding layoutBinding{
				.binding = resource.Binding,
				.descriptorType = GetResourceType(resource.Type),
				.descriptorCount = resource.ArrayCount,
				.stageFlags = GetShaderStage(resource.Stages),
				.pImmutableSamplers = nullptr,
			};
			vkLayouts.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.bindingCount = vkLayouts.size();
		createInfo.pBindings = vkLayouts.data();

		VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &mLayout);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create descriptor set layout");
			return;
		}
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		vkDestroyDescriptorSetLayout(device, mLayout, nullptr);
	}
}