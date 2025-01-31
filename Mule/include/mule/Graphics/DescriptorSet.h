#pragma once

#include "WeakRef.h"
#include "RenderTypes.h"
#include "Buffer/UniformBuffer.h"
#include "DescriptorSetLayout.h"

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
		uint32_t Binding;
		uint32_t ArrayElement;
		DescriptorType Type;
		std::vector<WeakRef<UniformBuffer>> Buffers;
	};

	class DescriptorSet
	{
	public:
		DescriptorSet(WeakRef<GraphicsContext> context, const DescriptorSetDescription& description);
		~DescriptorSet();

		void Update(const std::vector<DescriptorSetUpdate>& updates);

		VkDescriptorSet GetDescriptorSet() const { return mDescriptorSet; }
	private:
		VkDevice mDevice;
		VkDescriptorPool mDescriptorPool;
		VkDescriptorSet mDescriptorSet;
	};
}