#pragma once

namespace Mule
{
	enum class ResourceType {
		UniformBuffer,
		ShaderResourceGroup,
		Texture,
		CommandBuffer,
		Fence,
		TimelineSemaphore,
		CommandAllocator,
		RenderTarget,
		DepthAttachment,
		Sampler,
		DynamicVertexBuffer,
		DynamicIndexBuffer
	};
}