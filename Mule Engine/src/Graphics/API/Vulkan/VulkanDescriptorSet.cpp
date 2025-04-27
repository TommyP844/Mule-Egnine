#include "Graphics/API/Vulkan/VulkanDescriptorSet.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"
#include "Graphics/API/Vulkan/Buffer/VulkanUniformBuffer.h"

#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	VulkanDescriptorSet::VulkanDescriptorSet(const std::vector<Ref<ShaderResourceBlueprint>>& blueprints)
		:
		mDescriptorSet(VK_NULL_HANDLE)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		mDescriptorPool = context.GetDescriptorPool();

		std::vector<VkDescriptorSetLayout> vkLayouts;
		for (const auto& blueprint : blueprints)
		{
			Ref<VulkanDescriptorSetLayout> layout = blueprint;
			vkLayouts.push_back(layout->GetLayout());
		}

		VkDescriptorSetAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = vkLayouts.size();
		allocInfo.pSetLayouts = vkLayouts.data();

		VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &mDescriptorSet);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to allocate descriptor set");
			return;
		}
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkFreeDescriptorSets(device, mDescriptorPool, 1, &mDescriptorSet);
	}

	void VulkanDescriptorSet::Update(uint32_t binding, WeakRef<Texture> texture, uint32_t arrayIndex)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		WeakRef<IVulkanTexture> vkImage = texture;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = context.GetLinearSampler();
		imageInfo.imageView = vkImage->GetVulkanImage().ImageView;
		imageInfo.imageLayout = vkImage->GetVulkanImage().Layout;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = mDescriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = arrayIndex;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;		

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	// TODO: Texture view needs an image layout
	void VulkanDescriptorSet::Update(uint32_t binding, WeakRef<TextureView> texture, uint32_t arrayIndex)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		WeakRef<VulkanTextureView> vkImage = texture;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = context.GetLinearSampler();
		imageInfo.imageView = vkImage->GetView();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = mDescriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = arrayIndex;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	void VulkanDescriptorSet::Update(uint32_t binding, WeakRef<UniformBuffer> buffer, uint32_t arrayIndex)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		WeakRef<VulkanUniformBuffer> vkBuffer = buffer;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vkBuffer->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = buffer->GetSize();

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = mDescriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = arrayIndex;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pImageInfo = nullptr;
		write.pBufferInfo = &bufferInfo;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}
}