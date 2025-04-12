#pragma once

#include "WeakRef.h"
#include "VulkanRenderTypes.h"
#include "VulkanUniformBuffer.h"
#include "VulkanDescriptorSetLayout.h"
#include "IVulkanTexture.h"

#include <vector>

namespace Mule
{
	class GraphicsContext;

	class DescriptorSetUpdate
	{
	public:
		DescriptorSetUpdate() = default;
		DescriptorSetUpdate(const DescriptorSetUpdate& other) = default;
		explicit DescriptorSetUpdate(uint32_t binding, DescriptorType type, uint32_t arrayElement, const std::vector<WeakRef<IVulkanTexture>>& textures,
				const std::vector<WeakRef<VulkanUniformBuffer>>& buffers)
			:
			mBinding(binding),
			mArrayElement(arrayElement),
			mType(type),
			mBuffers(buffers)
		{
			for (auto texture : textures)
			{
				mTextures.push_back(texture->GetImageView());
				mLayouts.push_back(texture->GetVulkanImage().Layout);
				mSamplers.push_back(texture->GetSampler());
			}
		}

		explicit DescriptorSetUpdate(uint32_t binding, DescriptorType type, uint32_t arrayElement, VkImageView texture, VkSampler sampler, VkImageLayout layout)
			:
			mBinding(binding),
			mArrayElement(arrayElement),
			mType(type)
		{
			mTextures.push_back(texture);
			mSamplers.push_back(sampler);
			mLayouts.push_back(layout);
		}

		uint32_t GetBinding() const { return mBinding; }
		uint32_t GetArrayElement() const { return mArrayElement; }
		DescriptorType GetType() const { return mType; }
		const std::vector<WeakRef<VulkanUniformBuffer>>& GetBuffers() const { return mBuffers; }
		const std::vector<VkImageView>& GetImageViews() const { return mTextures; };
		const std::vector<VkSampler>& GetSamplers() const { return mSamplers; };
		const std::vector<VkImageLayout>& GetLayouts() const { return mLayouts; };
	private:
		uint32_t mBinding;
		uint32_t mArrayElement;
		DescriptorType mType;
		std::vector<WeakRef<VulkanUniformBuffer>> mBuffers;
		std::vector<VkImageView> mTextures;
		std::vector<VkSampler> mSamplers;
		std::vector<VkImageLayout> mLayouts;
	};

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet(const std::vector<WeakRef<VulkanDescriptorSetLayout>>& layouts);
		~VulkanDescriptorSet();

		void Update(const std::vector<DescriptorSetUpdate>& updates);

		VkDescriptorSet GetDescriptorSet() const { return mDescriptorSet; }
		const std::vector<WeakRef<VulkanDescriptorSetLayout>>& GetDescriptorSetLayouts() const { return mDescriptorSetLayouts; }
	private:
		WeakRef<GraphicsContext> mContext;
		VkDevice mDevice;
		VkDescriptorPool mDescriptorPool;
		VkDescriptorSet mDescriptorSet;
		std::vector<WeakRef<VulkanDescriptorSetLayout>> mDescriptorSetLayouts;
	};
}