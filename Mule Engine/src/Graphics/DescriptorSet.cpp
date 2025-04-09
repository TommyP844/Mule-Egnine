#include "Graphics/DescriptorSet.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	DescriptorSet::DescriptorSet(WeakRef<GraphicsContext> context, const std::vector<WeakRef<DescriptorSetLayout>>& layouts)
		:
		mContext(context),
		mDevice(context->GetDevice()),
		mDescriptorSet(VK_NULL_HANDLE),
		mDescriptorPool(context->GetDescriptorPool()),
		mDescriptorSetLayouts(layouts)
	{
		std::vector<VkDescriptorSetLayout> vkLayouts;
		for (const auto& setLayout : layouts)
		{
			vkLayouts.push_back(setLayout->GetLayout());
		}

		VkDescriptorSetAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = vkLayouts.size();
		allocInfo.pSetLayouts = vkLayouts.data();

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

			switch (update.GetType())
			{
			case DescriptorType::UniformBuffer:
			{
				count = update.GetBuffers().size();
				for (const auto& buffer : update.GetBuffers())
				{
					VkDescriptorBufferInfo bufferInfo{};

					bufferInfo.buffer = buffer->GetBuffer();
					bufferInfo.offset = 0;
					bufferInfo.range = VK_WHOLE_SIZE;

					bufferInfos.push_back(bufferInfo);
				}
			}
			break;
			case DescriptorType::StorageImage:
			case DescriptorType::Texture:
			{
				count = update.GetImageViews().size();
				for (uint32_t i = 0; i < count; i++)
				{
					VkDescriptorImageInfo imageInfo{};

					imageInfo.imageLayout = update.GetLayouts()[i];
					imageInfo.imageView = update.GetImageViews()[i];
					imageInfo.sampler = update.GetSamplers()[i];

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
			write.dstBinding = update.GetBinding();
			write.dstArrayElement = update.GetArrayElement();
			write.descriptorCount = count;
			write.descriptorType = (VkDescriptorType)update.GetType();
			write.pImageInfo = imageInfoGlobal.back().data();
			write.pBufferInfo = bufferInfoGlobal.back().data();
			write.pTexelBufferView = nullptr;
			writes.push_back(write);
		}

		vkUpdateDescriptorSets(mDevice, writes.size(), writes.data(), 0, nullptr);
	}
}