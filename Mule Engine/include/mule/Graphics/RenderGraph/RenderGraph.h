#pragma once

#include "Ref.h"

#include "Services/ServiceManager.h"

#include "Graphics/API/GraphicsQueue.h"
#include "Graphics/API/CommandAllocator.h"

// Resources
#include "Graphics/RenderGraph/ResourceHandle.h"
#include "Graphics/API/Texture.h"
#include "Graphics/API/UniformBuffer.h"
#include "Graphics/API/Framebuffer.h"
#include "Graphics/API/Semaphore.h"

#include <vector>

namespace Mule
{
	class Scene;
}

namespace Mule::RenderGraph
{
	class IRenderPass;

	class RenderGraph
	{
	public:
		RenderGraph(Ref<ServiceManager> serviceManager);
		virtual ~RenderGraph();

		// TODO: add factory system because almost all graph resources use a static Create() method for creation not constructors that MakeRef<T> takes
		template<class T, typename... Args>
		ResourceHandle<T> AddResource(Args&&... args);

		template<class T>
		Ref<T> GetResource(ResourceHandle<T> handle) const;

		void Execute(WeakRef<Scene> scene);
		Ref<Semaphore> GetCurrentSemaphore();

		virtual Ref<Framebuffer> GetCurrentFrameBuffer() const = 0;
		
		void Resize(uint32_t width, uint32_t height);
		
	protected:
		void Bake();

		template<typename T>
		WeakRef<T> CreatePass();

		void SetResizeCallback(std::function<void(uint32_t, uint32_t)> func);

	private:
		Ref<CommandAllocator> mCommandAllocator;
		Ref<GraphicsQueue> mQueue;
		Ref<ServiceManager> mServiceManager;
		std::vector<Ref<IRenderPass>> mPassesToCompile;
		uint32_t mFrameIndex;
		uint32_t mFramesInFlight;
		std::function<void(uint32_t, uint32_t)> mResizeCallback = nullptr;

		struct PassInFlight
		{
			Ref<CommandBuffer> Cmd							= nullptr;
			Ref<Fence> Fence								= nullptr;
			std::vector<Ref<Semaphore>> WaitSemaphores		= {};
			std::vector<Ref<Semaphore>> SignalSemaphores	= {};
			Ref<IRenderPass> Pass							= nullptr;
		};

		std::vector<PassInFlight> mPasses; 

		struct ResizeEvent
		{
			bool Resize = false;
			uint32_t Width = 0;
			uint32_t Height = 0;
		};
		std::vector<ResizeEvent> mResizeEvents;

		template<typename T>
		struct InFlightResource
		{
			std::vector<Ref<T>> Resources;
		};

		template<typename T>
		using ResourceMap = std::unordered_map<ResourceHandle<T>, InFlightResource<T>>;

		ResourceMap<Texture> mTextureResources;
		ResourceMap<UniformBuffer> mUniformBufferResources;
		ResourceMap<Framebuffer> mFramebufferResources;
		ResourceMap<ShaderResourceGroup> mShaderResourceGroupResources;

		template<typename T>
		const ResourceMap<T>& GetResourceMap() const;
		
		template<typename T>
		ResourceMap<T>& GetResourceMap();

		template<> const ResourceMap<Texture>& GetResourceMap() const { return mTextureResources; }
		template<> const ResourceMap<UniformBuffer>& GetResourceMap() const { return mUniformBufferResources; }
		template<> const ResourceMap<Framebuffer>& GetResourceMap() const { return mFramebufferResources; }
		template<> const ResourceMap<ShaderResourceGroup>& GetResourceMap() const { return mShaderResourceGroupResources; }

		template<> ResourceMap<Texture>& GetResourceMap() { return mTextureResources; }
		template<> ResourceMap<UniformBuffer>& GetResourceMap() { return mUniformBufferResources; }
		template<> ResourceMap<Framebuffer>& GetResourceMap() { return mFramebufferResources; }
		template<> ResourceMap<ShaderResourceGroup>& GetResourceMap() { return mShaderResourceGroupResources; }
	};
}

#include "RenderGraph.inl"