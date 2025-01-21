#pragma once

#include "WeakRef.h"
#include "RenderTypes.h"
#include "Buffer/UniformBuffer.h"

#include <vector>

namespace Mule
{
	class GraphicsContext;

	struct DescriptorLayoutDescription
	{
		uint32_t Binding;
		DescriptorType Type;
		ShaderStage Stage;
		uint32_t ArrayCount = 1;
	};

	struct DescriptorSetDescription
	{
		std::vector<DescriptorLayoutDescription> Layouts;
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
		VkDescriptorSetLayout mDescriptorSetLayout;
	};
}