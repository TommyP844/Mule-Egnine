#include "Graphics/renderer/RenderGraph/ResourceBuilder.h"

namespace Mule
{	
	ResourceBuilder::ResourceBuilder()
	{
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

	ResourceHandle ResourceBuilder::CreateSRG(const std::string& name, const std::vector<ShaderResourceDescription>& resources)
	{
		mSRGBlueprints[name] = SRGBlueprint {resources};
		return ResourceHandle(name, ResourceType::ShaderResourceGroup);
	}
}