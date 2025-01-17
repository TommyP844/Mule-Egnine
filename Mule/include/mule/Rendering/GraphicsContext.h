#pragma once

#include "Ref.h"
#include "WeakRef.h"
#include "Application/Window.h"

#include "GraphicsQueue.h"
#include "RenderPass.h"
#include "Fence.h"
#include "Semaphore.h"
#include "FrameBuffer.h"
#include "SwapchainframeBuffer.h"

#include <vulkan/vulkan.h>

namespace Mule
{
	struct GraphicsContextDescription
	{
		std::string AppName;
		std::string EngineName;
		uint32_t AppVersion;
		uint32_t EngineVersion;
		Ref<Window> Window = nullptr;
		bool EnableDebug = false;
	};

	struct PhysicalDeviceInfo
	{
		std::string Name;
		uint32_t DriverVersion;
		uint32_t DeviceId;
		uint32_t VendorId;
	};

	class GraphicsContext
	{
	public:
		GraphicsContext(const GraphicsContextDescription& description);
		~GraphicsContext();
		
		GraphicsContext(const GraphicsContext&) = delete;
		
		void BeginFrame();
		void EndFrame(std::vector<WeakRef<Semaphore>> gpuFences = {});
		void ResizeSwapchain(uint32_t width, uint32_t height);
		void WaitForDeviceIdle();

		Ref<RenderPass> CreateRenderPass(const RenderPassDescription& renderPassDescription);
		Ref<Fence> CreateFence();
		Ref<Semaphore> CreateSemaphore();
		Ref<FrameBuffer> CreateFrameBuffer(const FramebufferDescription& frameBufferDesc);


		// TODO: generate queues in a better manner, right now all three are identical
		WeakRef<GraphicsQueue> GetGraphicsQueue() const { return mGraphicsQueue; }
		WeakRef<GraphicsQueue> GetTransferQueue() const { return mTransferQueue; }
		WeakRef<GraphicsQueue> GetComputeQueue() const { return mComputeQueue; }

		WeakRef<Window> GetWindow() const { return mWindow; }
		const PhysicalDeviceInfo& GetPhysicalDeviceInfo() const { return mPhysicalDeviceInfo; }

		VkInstance GetInstance() const { return mInstance; }
		VkDevice GetDevice() const { return mDevice; }
		VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
		VkDescriptorPool GetDescriptorPool() const { return mDescriptorPool; }

		// Frame Data
		uint32_t GetImageIndex() const { return mImageIndex; }
		WeakRef<Fence> GetImageAcquiredCPUFence() const { return mFrameData[mFrameIndex].ImageAcquiredFence; }
		WeakRef<Semaphore> GetImageAcquiredGPUFence() const { return mFrameData[mFrameIndex].ImageAcquiredSemaphore; }

		Ref<SwapchainFrameBuffer> GetSwapchainFrameBuffer() const { return mFrameData[mFrameIndex].SwapchainFrameBuffer; }

		// Internal
		VkImageView CreateImageView(VkImage image, VkImageViewType viewtype, VkFormat format, int layers, int mips, bool depthImage);
		VulkanImage CreateImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageType imageType, int layers, int mips, VkImageUsageFlagBits usage);
	private:
		VkInstance mInstance;
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkSurfaceKHR mSurface;
		VkSwapchainKHR mSwapchain;
		VkDescriptorPool mDescriptorPool;
		PhysicalDeviceInfo mPhysicalDeviceInfo;
		VkDebugUtilsMessengerEXT mDebugMessenger;
		VkSurfaceFormatKHR mSurfaceFormat;
		VkCompositeAlphaFlagBitsKHR mCompositeAlphaFlags;
		VkPhysicalDeviceMemoryProperties mMemoryProperties;

		Ref<GraphicsQueue> mGraphicsQueue;
		Ref<GraphicsQueue> mComputeQueue;
		Ref<GraphicsQueue> mTransferQueue;
		
		// VkAllocationCallbacks mAllocCallback; TODO: implement

		Ref<Window> mWindow;

		// frame Data;
		uint32_t mFrameCount;
		uint32_t mFrameIndex;
		uint32_t mImageIndex;
		uint32_t mImageCount;

		struct FrameData
		{
			Ref<Fence> ImageAcquiredFence;
			Ref<Semaphore> ImageAcquiredSemaphore;
			Ref<SwapchainFrameBuffer> SwapchainFrameBuffer;
			Ref<RenderPass> SwapchainRenderPass;
		};
		std::vector<FrameData> mFrameData;

	};
}