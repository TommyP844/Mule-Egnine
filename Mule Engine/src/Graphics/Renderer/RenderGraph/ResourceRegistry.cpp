#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"

namespace Mule
{
	ResourceRegistry::ResourceRegistry(uint32_t framesInFlight, const ResourceBuilder& builder)
		:
		mFramesInFlight(framesInFlight),
		mFrameIndex(0),
		mOutputHandleLayer(0)
	{
		mResizeRequests.resize(mFramesInFlight);

		InFlightResource commandAllocator(mFramesInFlight);
		InFlightResource timelineSemaphore(mFramesInFlight);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			commandAllocator.Resources[i] = CommandAllocator::Create();
			timelineSemaphore.Resources[i] = TimelineSemaphore::Create();
			mResizeRequests[i].Handled = false;
			mResizeRequests[i].Width = 800;
			mResizeRequests[i].Height = 600;
		}

		mCommandAllocatorHandle = ResourceHandle("CommandAllocator", ResourceType::CommandAllocator);
		mTimelineSemaphoreHandle = ResourceHandle("TimelineSemaphore", ResourceType::TimelineSemaphore);

		mResources[mCommandAllocatorHandle] = commandAllocator;
		mResources[mTimelineSemaphoreHandle] = timelineSemaphore;

		for (const auto& [name, samplerBlueprint] : builder.GetSamplerBlueprints())
		{
			Ref<Sampler> sampler = Sampler::Create(samplerBlueprint.Description);
			InFlightResource SamplerResource(mFramesInFlight);

			for (uint32_t i = 0; i < mFramesInFlight; i++)
			{
				SamplerResource.Resources[i] = sampler;
			}

			ResourceHandle handle = ResourceHandle(name, ResourceType::Sampler);
			mResources[handle] = SamplerResource;
			mResourceHandles.push_back(handle);
		}

		for (const auto& [name, UBBlueprint] : builder.GetUniformBufferBlueprints())
		{
			InFlightResource UBIFR(mFramesInFlight);
			for (uint32_t i = 0; i < mFramesInFlight; i++)
			{
				UBIFR.Resources[i] = UniformBuffer::Create(UBBlueprint.Size);
			}

			ResourceHandle handle = ResourceHandle(name, ResourceType::UniformBuffer);
			mResources[handle] = UBIFR;
			mResourceHandles.push_back(handle);
		}

		for (const auto& [name, TextureBlueprints] : builder.GetTextureBlueprints())
		{
			InFlightResource TextureIFR(mFramesInFlight);
			for (uint32_t i = 0; i < mFramesInFlight; i++)
			{
				TextureIFR.Resources[i] = (Ref<Texture>)Texture2D::Create(name, {}, 800, 600, TextureBlueprints.format, TextureBlueprints.flags);
			}

			ResourceHandle handle = ResourceHandle(name, TextureBlueprints.Type);
			mResources[handle] = TextureIFR;
			mResourceHandles.push_back(handle);
		}

		for (const auto& [name, TextureBlueprints] : builder.GetTexture2DArrayBlueprints())
		{
			InFlightResource TextureIFR(mFramesInFlight);
			for (uint32_t i = 0; i < mFramesInFlight; i++)
			{
				TextureIFR.Resources[i] = (Ref<Texture>)Texture2DArray::Create(name, {}, 800, 600, TextureBlueprints.Layers, TextureBlueprints.Format, TextureBlueprints.Flags);
			}

			ResourceHandle handle = ResourceHandle(name, TextureBlueprints.Type);
			mResources[handle] = TextureIFR;
			mResourceHandles.push_back(handle);
		}

		for (const auto& [name, SRGBlueprint] : builder.GetSRGBlueprints())
		{
			InFlightResource SRGIFR(mFramesInFlight);
			for (uint32_t i = 0; i < mFramesInFlight; i++)
			{
				SRGIFR.Resources[i] = ShaderResourceGroup::Create(SRGBlueprint.Descriptions);
			}

			ResourceHandle handle = ResourceHandle(name, ResourceType::ShaderResourceGroup);
			mResources[handle] = SRGIFR;
			mResourceHandles.push_back(handle);
		}

	}

	ResourceRegistry::~ResourceRegistry()
	{
		for (const auto& handle : mResourceHandles)
		{
			if (handle.Type == ResourceType::CommandBuffer)
			{
				mResources.erase(handle);
			}
		}
	}

	ResourceHandle ResourceRegistry::AddFence(const std::string& name)
	{
		ResourceHandle handle = ResourceHandle(name, ResourceType::Fence);

		InFlightResource inFlightResource;

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			inFlightResource.Resources.push_back(Fence::Create());
		}

		mResources[handle] = inFlightResource;
		mFences.push_back(inFlightResource);
		mResourceHandles.push_back(handle);

		return handle;
	}

	ResourceHandle ResourceRegistry::AddCommandBuffer(const std::string& name)
	{
		InFlightResource commandBuffer(mFramesInFlight);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			auto commandAllocator = GetResource<CommandAllocator>(mCommandAllocatorHandle, i);
			commandBuffer.Resources[i] = commandAllocator->CreateCommandBuffer();
		}

		ResourceHandle handle = ResourceHandle(name, ResourceType::CommandBuffer);

		mResources[handle] = commandBuffer;
		mResourceHandles.push_back(handle);

		return handle;
	}

	WeakRef<TextureView> ResourceRegistry::GetColorOutput() const
	{
		if (mOutputHandle)
		{
			auto image = GetResource<Texture>(mOutputHandle, mFrameIndex);
			return image->GetView(0, mOutputHandleLayer);
		}

		return nullptr;
	}

	void ResourceRegistry::SetOutputHandle(ResourceHandle outputHandle, uint32_t layer)
	{
		assert(outputHandle.Type != ResourceType::RenderTarget || outputHandle.Type != ResourceType::DepthAttachment && "Output must be a texture 2d");

		mOutputHandle = outputHandle;
		mOutputHandleLayer = layer;
	}

	void ResourceRegistry::CopyRegistryResources(ResourceRegistry& registry)
	{
		for (auto& [handle, res] : registry.mResources)
		{
			mResources[handle] = res;
			mResourceHandles.push_back(handle);
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

	void ResourceRegistry::Resize(uint32_t width, uint32_t height)
	{
		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mResizeRequests[i].Handled = false;
			mResizeRequests[i].ResizeWidth = width;
			mResizeRequests[i].ResizeHeight = height;
		}
	}

	bool ResourceRegistry::IsResizeRequested(uint32_t frameIndex)
	{
		return !mResizeRequests[frameIndex].Handled;
	}

	void ResourceRegistry::SetResizeHandled(uint32_t frameIndex)
	{
		mResizeRequests[frameIndex].Handled = true;
		mResizeRequests[frameIndex].Width = mResizeRequests[frameIndex].ResizeWidth;
		mResizeRequests[frameIndex].Height = mResizeRequests[frameIndex].ResizeHeight;
	}

	std::pair<uint32_t, uint32_t> ResourceRegistry::GetResizeDimensions(uint32_t frameIndex)
	{
		return { mResizeRequests[frameIndex].ResizeWidth, mResizeRequests[frameIndex].ResizeHeight };
	}

	Ref<TimelineSemaphore> ResourceRegistry::GetSemaphore(uint32_t frameIndex) const
	{
		return GetResource<TimelineSemaphore>(mTimelineSemaphoreHandle, frameIndex);
	}
	
	uint32_t ResourceRegistry::GetWidth(uint32_t frameIndex) const
	{
		return mResizeRequests[frameIndex].Width;
	}

	uint32_t ResourceRegistry::GetHeight(uint32_t frameIndex) const
	{
		return mResizeRequests[frameIndex].Height;
	}

}
