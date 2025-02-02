#include "Graphics/DescriptorSet.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	DescriptorSet::DescriptorSet(WeakRef<GraphicsContext> context, const DescriptorSetDescription& description)
		:
		mContext(context),
		mDevice(context->GetDevice()),
		mDescriptorSet(VK_NULL_HANDLE),
		mDescriptorPool(context->GetDescriptorPool())
	{
		std::vector<VkDescriptorSetLayout> layouts;
		for (const auto& setLayout : description.Layouts)
		{
			layouts.push_back(setLayout->GetLayout());
		}

		VkDescriptorSetAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = layouts.size();
		allocInfo.pSetLayouts = layouts.data();

		VkResult result = vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to allocate descriptor set");
			return;
		}
	}

	DescriptorSet::~DescriptorSet()
	{
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
			case DescriptorType::Texture:
			{
				count = update.Textures.size();
				for (auto& texture : update.Textures)
				{
					VkDescriptorImageInfo imageInfo{};

					imageInfo.imageLayout = texture->GetVulkanImage().Layout;
					imageInfo.imageView = texture->GetImageView();
					imageInfo.sampler = mContext->GetLinearSampler();

					imageInfos.push_back(imageInfo);
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