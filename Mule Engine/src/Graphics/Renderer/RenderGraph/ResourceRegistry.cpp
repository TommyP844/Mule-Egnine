#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"

namespace Mule
{
	ResourceRegistry::ResourceRegistry(uint32_t framesInFlight, ResourceRegistryFlags flags)
		:
		mFramesInFlight(framesInFlight),
		mFlags(flags)
	{
		if (flags == ResourceRegistryFlags::CreateCommandAllocator)
		{
			mCommandAllocatorHandle = ResourceHandle::Create("CommandAllocator");

			InFlightResource inFlightResource;

			for (uint32_t i = 0; i < mFramesInFlight; i++)
			{
				inFlightResource.Resources.push_back(CommandAllocator::Create());
			}

			mResources[mCommandAllocatorHandle] = inFlightResource;
		}
	}

	ResourceHandle ResourceRegistry::AddFence(const std::string& name)
	{
		ResourceHandle handle = ResourceHandle::Create(name);

		InFlightResource inFlightResource;

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			inFlightResource.Resources.push_back(Fence::Create());
		}

		mResources[handle] = inFlightResource;
		mFences.push_back(inFlightResource);

		return handle;
	}

	ResourceHandle ResourceRegistry::AddCommandBuffer(const std::string& name)
	{
		assert(mFlags == ResourceRegistryFlags::CreateCommandAllocator && "ResoureRegistry was not created with CreateCommandBufferFlag");

		ResourceHandle handle = ResourceHandle::Create(name);

		InFlightResource inFlightResource;

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			Ref<CommandAllocator> allocator = GetResource<CommandAllocator>(mCommandAllocatorHandle, i);
			inFlightResource.Resources.push_back(allocator->CreateCommandBuffer());
		}

		mResources[handle] = inFlightResource;

		return handle;
	}

	Ref<Texture2D> ResourceRegistry::GetColorOutput(uint32_t frameIndex) const
	{
		Ref<Framebuffer> framebuffer = GetResource<Framebuffer>(mFramebufferOutputHandle, frameIndex);
		return framebuffer->GetColorAttachment(0);
	}

	void ResourceRegistry::CopyRegistryResources(ResourceRegistry& registry)
	{
		for (auto& [handle, res] : registry.mResources)
		{
			mResources[handle] = res;
		}
	}

	void ResourceRegistry::WaitForFences(uint32_t frameIndex)
	{
		for (auto fenceResource : mFences)
		{
			auto fence = std::get<Ref<Fence>>(fenceResource.Resources[frameIndex]);
			fence->Wait();
		}
	}
}
