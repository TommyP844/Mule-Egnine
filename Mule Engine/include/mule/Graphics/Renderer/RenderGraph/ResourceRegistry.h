#pragma once

#include "Ref.h"
#include "Graphics/Renderer/RenderGraph/ResourceHandle.h"
#include "Graphics/Renderer/RenderGraph/ResourceType.h"
#include "Graphics/Renderer/RenderGraph/ResourceBuilder.h"

// Resources
#include "Graphics/API/Texture.h"
#include "Graphics/API/UniformBuffer.h"
#include "Graphics/API/Framebuffer.h"
#include "Graphics/API/Semaphore.h"
#include "Graphics/API/ShaderResourceGroup.h"
#include "Graphics/API/Fence.h"
#include "Graphics/API/CommandAllocator.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/API/TimelineSemaphore.h"
#include "Graphics/API/Texture2DArray.h"
#include "Graphics/API/DynamicVertexBuffer.h"
#include "Graphics/API/DynamicIndexBuffer.h"

#include <vector>
#include <variant>

namespace Mule
{
	enum class RegistryVariable : uint32_t {
		Width,
		Height,

		MAX
	};

	class ResourceRegistry
	{
	public:
		ResourceRegistry(uint32_t framesInFlight, const ResourceBuilder& builder);

		~ResourceRegistry();

		template<class T, typename... Args>
		void AddResource(const std::string& name, ResourceType type, Args&&... args);

		template<class T>
		void InsertResources(ResourceHandle handle, const std::vector<Ref<T>>& resources);

		ResourceHandle AddFence(const std::string& name);
		ResourceHandle AddCommandBuffer(const std::string& name);

		template<class T>
		Ref<T> GetResource(ResourceHandle handle, uint32_t frameIndex) const;

		WeakRef<TextureView> GetColorOutput() const;
		ResourceHandle GetColorOutputHandle() const { return mOutputHandle; }
		uint32_t GetFramesInFlight() const { return mFramesInFlight; }

		void SetOutputHandle(ResourceHandle outputHandle, uint32_t layer = 0);
		void CopyRegistryResources(ResourceRegistry& registry);
		void WaitForFences(uint32_t frameIndex);

		void Resize(uint32_t width, uint32_t height);
		bool IsResizeRequested(uint32_t frameIndex);
		void SetResizeHandled(uint32_t frameIndex);
		std::pair<uint32_t, uint32_t> GetResizeDimensions(uint32_t frameIndex);

		Ref<TimelineSemaphore> GetSemaphore(uint32_t frameIndex) const;

		uint32_t GetWidth(uint32_t frameIndex) const;
		uint32_t GetHeight(uint32_t frameIndex) const;

		void SetFrameIndex(uint32_t frameIndex) { mFrameIndex = frameIndex; }

		const std::vector<ResourceHandle>& GetResourceHandles() const { return mResourceHandles; }

		template<typename T>
		T GetVariable(RegistryVariable var, uint32_t frameIndex = 0) const;

	private:
		uint32_t mFramesInFlight;
		uint32_t mFrameIndex;

		// Outputs
		uint32_t mAttachmentIndex = 0;
		bool mOutputIsDepth = false;

		ResourceHandle mOutputHandle;
		uint32_t mOutputHandleLayer;
		ResourceHandle mCommandAllocatorHandle;
		ResourceHandle mTimelineSemaphoreHandle;

		using ResourceVariant = std::variant<
			Ref<Texture>,
			Ref<UniformBuffer>,
			Ref<Framebuffer>,
			Ref<ShaderResourceGroup>,
			Ref<Fence>,
			Ref<CommandAllocator>,
			Ref<CommandBuffer>,
			Ref<TimelineSemaphore>,
			Ref<Sampler>,
			Ref<DynamicVertexBuffer>,
			Ref<DynamicIndexBuffer>
			>;

		struct InFlightResource
		{
			InFlightResource() = default;
			InFlightResource(uint32_t framesInFlight)
			{
				Resources.resize(framesInFlight);
			}

			std::vector<ResourceVariant> Resources;
		};

		using ResourceMap = std::unordered_map<ResourceHandle, InFlightResource>;
		std::vector<ResourceHandle> mResourceHandles;

		ResourceMap mResources;
		std::vector<InFlightResource> mFences;
		
		struct ResizeRequest {
			bool Handled = true;
			uint32_t ResizeWidth = 0;
			uint32_t ResizeHeight = 0;
			uint32_t Width = 0;
			uint32_t Height = 0;
		};

		std::vector<ResizeRequest> mResizeRequests;
	};
}

#include "ResourceRegistry.inl"