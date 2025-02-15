#pragma once

#include "WeakRef.h"
#include "RenderTypes.h"
#include "Buffer/UniformBuffer.h"
#include "DescriptorSetLayout.h"
#include "Graphics/Texture/ITexture.h"

#include <vector>

namespace Mule
{
	class GraphicsContext;

	struct DescriptorSetDescription
	{
		std::vector<WeakRef<DescriptorSetLayout>> Layouts;
	};

	struct DescriptorSetUpdate
	{
		DescriptorSetUpdate() = default;
		DescriptorSetUpdate(const DescriptorSetUpdate& other) = default;
		DescriptorSetUpdate(uint32_t binding, DescriptorType type, uint32_t arrayElement, const std::vector<WeakRef<ITexture>>& textures,
				const std::vector<WeakRef<UniformBuffer>>& buffers)
			:
			Binding(binding),
			ArrayElement(arrayElement),
			Type(type),
			Buffers(buffers),
			Textures(textures)
		{}
		uint32_t Binding;
		uint32_t ArrayElement;
		DescriptorType Type;
		std::vector<WeakRef<UniformBuffer>> Buffers;
		std::vector<WeakRef<ITexture>> Textures;
	};

	class DescriptorSet
	{
	public:
		DescriptorSet(WeakRef<GraphicsContext> context, const DescriptorSetDescription& description);
		~DescriptorSet();

		void Update(const std::vector<DescriptorSetUpdate>& updates);

		VkDescriptorSet GetDescriptorSet() const { return mDescriptorSet; }
		const std::vector<WeakRef<DescriptorSetLayout>>& GetDescriptorSetLayouts() const { return mDescriptorSetLayouts; }
	private:
		WeakRef<GraphicsContext> mContext;
		VkDevice mDevice;
		VkDescriptorPool mDescriptorPool;
		VkDescriptorSet mDescriptorSet;
		std::vector<WeakRef<DescriptorSetLayout>> mDescriptorSetLayouts;
	};
}