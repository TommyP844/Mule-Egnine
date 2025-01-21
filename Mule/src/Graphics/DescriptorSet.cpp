#include "Graphics/DescriptorSet.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	DescriptorSet::DescriptorSet(WeakRef<GraphicsContext> context, const DescriptorSetDescription& description)
		:
		mDevice(context->GetDevice()),
		mDescriptorSet(VK_NULL_HANDLE),
		mDescriptorSetLayout(VK_NULL_HANDLE),
		mDescriptorPool(context->GetDescriptorPool())
	{
		std::vector<VkDescriptorSetLayoutBinding> layouts;
		for(const auto& layout : description.Layouts)
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

		VkResult result = vkCreateDescriptorSetLayout(mDevice, &createInfo, nullptr, &mDescriptorSetLayout);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create descriptor set layout");
			return;
		}


		VkDescriptorSetAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayout;

		result = vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to allocate descriptor set");
			return;
		}
	}

	DescriptorSet::~DescriptorSet()
	{
		vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
		vkFreeDescriptorSets(mDevice, mDescriptorPool, 1, &mDescriptorSet);
	}

	void DescriptorSet::Update(const std::vector<DescriptorSetUpdate>& updates)
	{
		std::vector<VkWriteDescriptorSet> writes{};

		std::vector<std::vector<VkDescriptorImageInfo>> imageInfoGlobal;
		std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfoGlobal;
		for(const auto& update : updates)
		{
			std::vector<VkDescriptorImageInfo> imageInfos;
			std::vector<VkDescriptorBufferInfo> bufferInfos;
			uint32_t count = 0;

			switch (update.Type)
			{
			case DescriptorType::UniformBuffer:
			{
				count = update.Buffers.size();
				for (auto& buffer : update.Buffers)
				{
					VkDescriptorBufferInfo bufferInfo{};

					bufferInfo.buffer = buffer->GetBuffer();
					bufferInfo.offset = 0;
					bufferInfo.range = VK_WHOLE_SIZE;

					bufferInfos.push_back(bufferInfo);
				}
			}
				break;
			}

			imageInfoGlobal.push_back(imageInfos);
			bufferInfoGlobal.push_back(bufferInfos);

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = mDescriptorSet;
			write.dstBinding = update.Binding;
			write.dstArrayElement = update.ArrayElement;
			write.descriptorCount = count;
			write.descriptorType = (VkDescriptorType)update.Type;
			write.pImageInfo = imageInfoGlobal.back().data();
			write.pBufferInfo = bufferInfoGlobal.back().data();
			write.pTexelBufferView = nullptr;
			writes.push_back(write);
		}

		vkUpdateDescriptorSets(mDevice, writes.size(), writes.data(), 0, nullptr);
	}
}