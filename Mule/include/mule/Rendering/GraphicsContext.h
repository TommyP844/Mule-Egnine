#pragma once

#include "Ref.h"
#include "WeakRef.h"
#include "Application/Window.h"

#include "GraphicsQueue.h"


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
		void EndFrame();


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

	private:
		VkInstance mInstance;
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkSurfaceKHR mSurface;
		VkSwapchainKHR mSwapchain;
		VkDescriptorPool mDescriptorPool;
		PhysicalDeviceInfo mPhysicalDeviceInfo;
		VkDebugUtilsMessengerEXT mDebugMessenger;

		Ref<GraphicsQueue> mGraphicsQueue;
		Ref<GraphicsQueue> mComputeQueue;
		Ref<GraphicsQueue> mTransferQueue;
		
		// VkAllocationCallbacks mAllocCallback; TODO: implement

		Ref<Window> mWindow;
	};
}