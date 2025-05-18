#pragma once

#include "Graphics/API/ShaderResourceGroup.h"
#include "Graphics/API/Sampler.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanDescriptorSet : ShaderResourceGroup
	{
	public:
		VulkanDescriptorSet(Ref<ShaderResourceBlueprint> blueprint);
		~VulkanDescriptorSet();

		void Update(uint32_t binding, DescriptorType type, ImageLayout layout, WeakRef<Texture> texture, uint32_t arrayIndex = 0, Ref<Sampler> sampler = nullptr) override;
		void Update(uint32_t binding, DescriptorType type, ImageLayout layout, WeakRef<TextureView> texture, uint32_t arrayIndex = 0, Ref<Sampler> sampler = nullptr) override;
		void Update(uint32_t binding, WeakRef<UniformBuffer> buffer, uint32_t arrayIndex = 0) override;

		VkDescriptorSet GetDescriptorSet() const { return mDescriptorSet; }
	private:
		VkDescriptorPool mDescriptorPool;
		VkDescriptorSet mDescriptorSet;
	};
}