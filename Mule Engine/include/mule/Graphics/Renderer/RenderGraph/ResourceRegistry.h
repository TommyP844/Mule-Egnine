#pragma once

#include "Ref.h"
#include "Graphics/Renderer/RenderGraph/ResourceHandle.h"

// Resources
#include "Graphics/API/Texture.h"
#include "Graphics/API/UniformBuffer.h"
#include "Graphics/API/Framebuffer.h"
#include "Graphics/API/Semaphore.h"
#include "Graphics/API/ShaderResourceGroup.h"
#include "Graphics/API/Fence.h"
#include "Graphics/API/CommandAllocator.h"
#include "Graphics/API/CommandBuffer.h"

#include <vector>
#include <variant>

namespace Mule
{
	enum class ResourceRegistryFlags
	{
		None,
		CreateCommandAllocator
	};

	class ResourceRegistry
	{
	public:
		ResourceRegistry(uint32_t framesInFlight, ResourceRegistryFlags flags = ResourceRegistryFlags::CreateCommandAllocator);

		~ResourceRegistry() = default;

		template<class T, typename... Args>
		void AddResource(ResourceHandle handle, Args&&... args);

		ResourceHandle AddFence(const std::string& name);
		ResourceHandle AddCommandBuffer(const std::string& name);
				

		template<class T>
		Ref<T> GetResource(ResourceHandle handle, uint32_t frameIndex) const;

		Ref<Texture2D> GetColorOutput(uint32_t frameIndex) const;
		uint32_t GetFramesInFlight() const { return mFramesInFlight; }

		void SetOutputFramebufferHandle(ResourceHandle outputFramebuffer) { mFramebufferOutputHandle = outputFramebuffer; }
		void CopyRegistryResources(ResourceRegistry& registry);
		void WaitForFences(uint32_t frameIndex);

	private:
		ResourceRegistryFlags mFlags;
		uint32_t mFramesInFlight;

		ResourceHandle mFramebufferOutputHandle;
		ResourceHandle mCommandAllocatorHandle;

		using ResourceVariant = std::variant<
			Ref<Texture>,
			Ref<UniformBuffer>,
			Ref<Framebuffer>,
			Ref<Texture>,
			Ref<ShaderResourceGroup>,
			Ref<Fence>,
			Ref<CommandAllocator>,
			Ref<CommandBuffer>
			>;

		struct InFlightResource
		{
			std::vector<ResourceVariant> Resources;
		};

		using ResourceMap = std::unordered_map<ResourceHandle, InFlightResource>;

		ResourceMap mResources;

		std::vector<InFlightResource> mFences;
	};
}

#include "ResourceRegistry.inl"