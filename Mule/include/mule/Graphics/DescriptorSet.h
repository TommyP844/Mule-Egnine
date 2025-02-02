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
	private:
		WeakRef<GraphicsContext> mContext;
		VkDevice mDevice;
		VkDescriptorPool mDescriptorPool;
		VkDescriptorSet mDescriptorSet;
	};
}