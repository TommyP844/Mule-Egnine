#include "Graphics/API/Vulkan/VulkanDescriptorSet.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"
#include "Graphics/API/Vulkan/Buffer/VulkanUniformBuffer.h"
#include "Graphics/API/Vulkan/Texture/VulkanTextureCube.h"

#include "Graphics/API/Vulkan/VulkanTypeConversion.h"

#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	VulkanDescriptorSet::VulkanDescriptorSet(Ref<ShaderResourceBlueprint> blueprint)
		:
		mDescriptorSet(VK_NULL_HANDLE)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		mDescriptorPool = context.GetDescriptorPool();

		mBlueprint = blueprint;
		Ref<VulkanDescriptorSetLayout> layout = blueprint;
		VkDescriptorSetLayout vkLayout = layout->GetLayout();		

		VkDescriptorSetAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &vkLayout;

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

	void VulkanDescriptorSet::Update(uint32_t binding, DescriptorType type, WeakRef<Texture> texture, uint32_t arrayIndex)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		WeakRef<VulkanTextureCube> vkImage = texture;

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
		write.descriptorType = GetDescriptorType(type);
		write.pImageInfo = &imageInfo;
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;		

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	// TODO: Texture view needs an image layout
	void VulkanDescriptorSet::Update(uint32_t binding, DescriptorType type, ImageLayout layout, WeakRef<TextureView> texture, uint32_t arrayIndex)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();
		WeakRef<VulkanTextureView> vkImage = texture;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = context.GetLinearSampler();
		imageInfo.imageView = vkImage->GetView();
		imageInfo.imageLayout = GetImageLayout(layout);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = mDescriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = arrayIndex;
		write.descriptorCount = 1;
		write.descriptorType = GetDescriptorType(type);
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