#pragma once

#include "WeakRef.h"
#include "Application/Window.h"

#include "Texture/IVulkanTexture.h"
#include "Syncronization/VulkanFence.h"
#include "Syncronization/VulkanSemaphore.h"
#include "Execution/VulkanCommandBuffer.h"

#include <Volk/volk.h>

#include <unordered_map>
#include <mutex>
#include <stack>
#include <vector>

namespace Mule::Vulkan
{
	class IVulkanBuffer;

	class VulkanContext
	{
	public:
		static void Init(Ref<Window> window);
		static void Shutdown();
		static VulkanContext& Get();

		bool BeginFrame();
		void EndFrame(const std::vector<VkSemaphore>& waitSemaphores = {});
		void ResizeSwapchain(uint32_t width, uint32_t height);

		void AwaitIdle();

		WeakRef<Window> GetWindow() const;

		VkInstance GetInstance() const { return mInstance; }
		VkDevice GetDevice() const { return mDevice; }
		VkPhysicalDevice GetPhysicalDevice();
		VkDescriptorPool GetDescriptorPool() const { return mDescriptorPool; }
		uint32_t GetMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return mMemoryProperties; }
		VkSampler GetLinearSampler() const { return mLinearSampler; }
		VkFormat GetSurfaceFormat() const { return mSurfaceFormat.format; }
		
		uint32_t GetQueueFamilyIndex() const;
		VkQueue CreateQueue();
		void ReleaseQueue(VkQueue queue);
		
		Ref<VulkanCommandBuffer> BeginSingleTimeCommandBuffer();
		void EndSingleTimeCommandBuffer(Ref<VulkanCommandBuffer> cmd);
		void CopyBuffer(Ref<VulkanCommandBuffer> cmd, WeakRef<IVulkanBuffer> src, WeakRef<IVulkanBuffer> dst);
		void CopyBufferToImage(Ref<VulkanCommandBuffer> cmd, VkBuffer buffer, VkImage image, const VkBufferImageCopy& bufferImageCopy);
		void BlitMip(Ref<VulkanCommandBuffer> cmd, WeakRef<IVulkanTexture> texture, const VkImageBlit& blit);
		void TransitionImageLayout(Ref<VulkanCommandBuffer> cmd, WeakRef<IVulkanTexture> texture, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);

		VkImageView GetCurrentSwapchainColorImageView();
		VkImageView GetCurrentSwapchainDepthImageView();
		VkImage GetCurrentSwapchainColorImage();

	private:
		static VulkanContext* mVulkanContext;

		VulkanContext(Ref<Window> window);
		~VulkanContext();

		// On Context creation we create some vulkan objects that need to reference the context, so we want to be able to call the constructor and initialize seperatly
		void InitInternal();

		Ref<Window> mWindow;

		VkInstance mInstance;
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkSurfaceKHR mSurface;
		VkSwapchainKHR mSwapchain;
		VkDescriptorPool mDescriptorPool;
		VkDebugUtilsMessengerEXT mDebugMessenger;
		VkSurfaceFormatKHR mSurfaceFormat;
		VkCompositeAlphaFlagBitsKHR mCompositeAlphaFlags;
		VkPhysicalDeviceMemoryProperties mMemoryProperties;
		
		std::mutex mQueueMutex;
		std::mutex mCommandPoolMutex;
		VkQueue mContextQueue;

		std::unordered_map<std::thread::id, VkCommandPool> mContextCommandPools;
		
		void CreateMemoryBarrier(VkImageMemoryBarrier& barrier, WeakRef<IVulkanTexture> texture, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);
		VkAccessFlags GetAccessFlagsFromLayout(VkImageLayout layout);
		VkPipelineStageFlags GetPipelineStageFlagFromLayout(VkImageLayout layout);

		VkQueueFamilyProperties mQueueFamilyProperties;
		std::stack<uint32_t> mFreeQueueIndices;
		std::unordered_map<VkQueue, uint32_t> mAllocatedQueueIndices;
		uint32_t mQueueFamilyIndex;

		VkSampler mLinearSampler;

		// VkAllocationCallbacks mAllocCallback; TODO: implement

		
		// frame Data;
		uint32_t mFrameCount;
		uint32_t mFrameIndex;

		struct FrameData
		{
			VkImage ColorImage;
			VkImageView ColorImageView;
			VkImage DepthImage;
			VkDeviceMemory DepthImageMemory;
			VkImageView DepthImageView;
			Ref<VulkanFence> ImageAcquiredFence;
			Ref<VulkanSemaphore> ImageAcquiredSemaphore;
		};
		std::vector<FrameData> mFrameData;
	};
}
