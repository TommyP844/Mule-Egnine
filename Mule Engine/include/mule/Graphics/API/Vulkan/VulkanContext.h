#pragma once

#include "WeakRef.h"
#include "Application/Window.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanContext
	{
	public:
		static void Init(WeakRef<Window> window);
		static void Shutdown();
		static VulkanContext& Get();

		bool BeginFrame();
		void EndFrame(const std::vector<VkSemaphore>& waitSemaphores = {});
		void ResizeSwapchain(uint32_t width, uint32_t height);

		VkInstance GetInstance() const { return mInstance; }
		VkDevice GetDevice() const { return mDevice; }
		VkDescriptorPool GetDescriptorPool() const { return mDescriptorPool; }
		uint32_t GetMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return mMemoryProperties; }
		VkSampler GetLinearSampler() const { return mLinearSampler; }
		VkFormat GetSurfaceFormat() const { return mSurfaceFormat.format; }

	private:
		static VulkanContext* mVulkanContext;

		VulkanContext(WeakRef<Window> window);
		~VulkanContext();

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

		VkSampler mLinearSampler;

		// VkAllocationCallbacks mAllocCallback; TODO: implement

		Ref<Window> mWindow;

		// frame Data;
		uint32_t mFrameCount;
		uint32_t mFrameIndex;
		uint32_t mImageIndex;
		uint32_t mImageCount;

		struct FrameData
		{
			Ref<VulkanFence> ImageAcquiredFence;
			Ref<VulkanSemaphore> ImageAcquiredSemaphore;
			Ref<VulkanSwapchainFrameBuffer> SwapchainFrameBuffer;
		};
		std::vector<FrameData> mFrameData;
	};
}
