#include "Graphics/API/Vulkan/Texture/VulkanSampler.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include "Graphics/API/Vulkan/VulkanTypeConversion.h"

namespace Mule::Vulkan
{
	VulkanSampler::VulkanSampler(const SamplerDescription& description)
		:
		mSampler(VK_NULL_HANDLE)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.magFilter = GetFilter(description.MagFilterMode);
		createInfo.minFilter = GetFilter(description.MinFilterMode);
		createInfo.mipmapMode = GetMipMapMode(description.MipMapMode);
		createInfo.addressModeU = GetAddressMode(description.AddressModeU);
		createInfo.addressModeV = GetAddressMode(description.AddressModeV);
		createInfo.addressModeW = GetAddressMode(description.AddressModeW);
		createInfo.mipLodBias = 0.f;
		createInfo.anisotropyEnable = description.AnisotropyEnable;
		createInfo.maxAnisotropy = description.MaxAnisotropy;
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp; // Unused
		createInfo.minLod = description.MinLod;
		createInfo.maxLod = description.MaxLod;
		createInfo.borderColor = GetBorderColor(description.BorderColor);
		createInfo.unnormalizedCoordinates = VK_FALSE;

		vkCreateSampler(device, &createInfo, nullptr, &mSampler);
	}
	
	VulkanSampler::~VulkanSampler()
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		vkDestroySampler(device, mSampler, nullptr);
	}

	VkSampler VulkanSampler::GetSampler() const
	{
		return mSampler;
	}
}