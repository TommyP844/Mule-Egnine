#include "Graphics/renderer/RenderGraph/ResourceBuilder.h"

namespace Mule
{	
	ResourceBuilder::ResourceBuilder()
	{
	}

	ResourceHandle ResourceBuilder::CreateDynamicVertexBuffer(const std::string& name, const VertexLayout& layout, uint32_t vertexCount)
	{
		mDynamicVertexBufferBlueprints[name] = DynamicVertexBufferBlueprint{ layout, vertexCount };
		return ResourceHandle(name, ResourceType::DynamicVertexBuffer);
	}

	ResourceHandle ResourceBuilder::CreateDynamicIndexBuffer(const std::string& name, IndexType index, uint32_t vertexCount)
	{
		mDynamicIndexBufferBlueprints[name] = DynamicIndexBufferBlueprint{ index, vertexCount };
		return ResourceHandle(name, ResourceType::DynamicIndexBuffer);
	}

	ResourceHandle ResourceBuilder::CreateSampler(const std::string& name, const SamplerDescription& description)
	{
		mSamplerBlueprints[name] = SamplerBlueprint{ description };
		return ResourceHandle(name, ResourceType::Sampler);
	}

	ResourceHandle ResourceBuilder::CreateUniformBuffer(const std::string& name, uint32_t bufferSize)
	{
		mUniformBufferBlueprints[name] = UniformBufferBlueprint {bufferSize};
		return ResourceHandle(name, ResourceType::UniformBuffer);
	}

	ResourceHandle ResourceBuilder::CreateTexture2D(const std::string& name, TextureFormat format, TextureFlags flags)
	{
		ResourceType type = ResourceType::Texture;
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
			type = ResourceType::RenderTarget;
		else if ((flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment)
			type = ResourceType::DepthAttachment;

		mTexture2DBlueprints[name] = Texture2DBlueprint {format, flags, type};
		return ResourceHandle(name, type);
	}

	ResourceHandle ResourceBuilder::CreateTexture2DArray(const std::string& name, uint32_t layers, TextureFormat format, TextureFlags flags)
	{
		ResourceType type = ResourceType::Texture;
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
			type = ResourceType::RenderTarget;
		else if ((flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment)
			type = ResourceType::DepthAttachment;

		mTexture2DArrayBlueprints[name] = Texture2DArrayBlueprint{ layers, format, flags, type };
		return ResourceHandle(name, type);
	}

	ResourceHandle ResourceBuilder::CreateSRG(const std::string& name, const std::vector<ShaderResourceDescription>& resources)
	{
		mSRGBlueprints[name] = SRGBlueprint {resources};
		return ResourceHandle(name, ResourceType::ShaderResourceGroup);
	}
}