#pragma once

#include "Graphics/API/ShaderResourceGroup.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanDescriptorSet : ShaderResourceGroup
	{
	public:
		VulkanDescriptorSet(const std::vector<Ref<ShaderResourceBlueprint>>& blueprints);
		~VulkanDescriptorSet();

		void Update(uint32_t binding, WeakRef<Texture> texture, uint32_t arrayIndex = 0) override;
		void Update(uint32_t binding, WeakRef<TextureView> texture, uint32_t arrayIndex = 0) override;
		void Update(uint32_t binding, WeakRef<UniformBuffer> buffer, uint32_t arrayIndex = 0) override;

		VkDescriptorSet GetDescriptorSet() const { return mDescriptorSet; }
	private:
		VkDescriptorPool mDescriptorPool;
		VkDescriptorSet mDescriptorSet;
	};
}