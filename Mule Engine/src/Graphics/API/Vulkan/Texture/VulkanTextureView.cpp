#include "Graphics/API/Vulkan/Texture/VulkanTextureView.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

namespace Mule::Vulkan
{
	VulkanTextureView::VulkanTextureView(VkImageView view)
		:
		mView(view)
	{
		VulkanContext& context = VulkanContext::Get();
		VkSampler sampler = context.GetLinearSampler();
		mImGuiID = (ImTextureID)ImGui_ImplVulkan_AddTexture(sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	VulkanTextureView::~VulkanTextureView()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkDestroyImageView(device, mView, nullptr);
	}
}