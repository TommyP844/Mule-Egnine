#include "Rendering/GraphicsContext.h"

// Submodules
#include "spdlog/spdlog.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"


// STD

namespace Mule
{
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		SPDLOG_INFO(pCallbackData->pMessage);

		// Return VK_FALSE to let Vulkan continue
		return VK_FALSE;
	}

	GraphicsContext::GraphicsContext(const GraphicsContextDescription& description)
        :
        mWindow(description.Window),
		mInstance(VK_NULL_HANDLE),
		mDevice(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mSwapchain(VK_NULL_HANDLE)
	{
		std::vector<const char*> enabledLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		std::vector<const char*> enabledExtensions = {
			"VK_EXT_debug_utils"
		};


		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (int i = 0; i < glfwExtensionCount; i++)
		{
			enabledExtensions.push_back(glfwExtensions[i]);
		}

#pragma region Instance

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_3;
		appInfo.applicationVersion = description.AppVersion;
		appInfo.engineVersion = description.EngineVersion;
		appInfo.pApplicationName = description.AppName.c_str();
		appInfo.pEngineName = description.EngineName.c_str();
		appInfo.pNext = nullptr;
 
		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.enabledExtensionCount = enabledExtensions.size();
		instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
		instanceInfo.enabledLayerCount = enabledLayers.size();
		instanceInfo.ppEnabledLayerNames = enabledLayers.data();
		instanceInfo.flags = 0;
		instanceInfo.pApplicationInfo = nullptr;
		  
		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &mInstance);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Vulkan initialized");
		}
		else
		{
			SPDLOG_ERROR("Vulkan failed to init, Code: {}", (uint32_t)result);
			exit(1);
		}
#pragma endregion

#pragma region Debug

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;

		auto createDebugUtilsMessengerEXT =	(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
		if (createDebugUtilsMessengerEXT) 
		{
			createDebugUtilsMessengerEXT(mInstance, &debugCreateInfo, nullptr, &mDebugMessenger);
		}
		else {
			SPDLOG_ERROR("Failed to set up debug messenger");
		}

#pragma endregion

#pragma region Physical Device
		uint32_t physicalDeviceCount = 0;
		std::vector<VkPhysicalDevice> physicalDevices;

		vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
		physicalDevices.resize(physicalDeviceCount);
		vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data());

		if (physicalDeviceCount == 0)
		{
			SPDLOG_ERROR("Failed to find physical devices that support vulkan");
			exit(1);
		}

		for (int i = 0; i < physicalDeviceCount; i++)
		{
			auto& physicalDevice = physicalDevices[i];
			VkPhysicalDeviceFeatures physicalDeviceFeatures{};
			VkPhysicalDeviceProperties phyicalDeviceProperties{};
			vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
			vkGetPhysicalDeviceProperties(physicalDevice, &phyicalDeviceProperties);
			
			if (phyicalDeviceProperties.deviceType == POINTER_DEVICE_TYPE_EXTERNAL_PEN)
			{
				physicalDeviceFeatures = physicalDeviceFeatures;
				mPhysicalDevice = physicalDevice;
			}
		}

		if (mPhysicalDevice == VK_NULL_HANDLE)
		{
			mPhysicalDevice = physicalDevices[0];
		}

		VkPhysicalDeviceProperties phyicalDeviceProperties;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;

		vkGetPhysicalDeviceProperties(mPhysicalDevice, &phyicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(mPhysicalDevice, &physicalDeviceFeatures);

		mPhysicalDeviceInfo.Name = phyicalDeviceProperties.deviceName;
		mPhysicalDeviceInfo.DriverVersion = phyicalDeviceProperties.driverVersion;
		mPhysicalDeviceInfo.DeviceId = phyicalDeviceProperties.deviceID;
		mPhysicalDeviceInfo.VendorId = phyicalDeviceProperties.vendorID;

		SPDLOG_INFO("Physical Device Selected");
		SPDLOG_INFO("\t Name           : {}", mPhysicalDeviceInfo.Name);
		SPDLOG_INFO("\t Driver Version : {}.{}.{}", VK_API_VERSION_MAJOR(mPhysicalDeviceInfo.DriverVersion), VK_API_VERSION_MINOR(mPhysicalDeviceInfo.DriverVersion), VK_API_VERSION_PATCH(mPhysicalDeviceInfo.DriverVersion));
		SPDLOG_INFO("\t Device ID      : {}.{}.{}", VK_API_VERSION_MAJOR(mPhysicalDeviceInfo.DeviceId), VK_API_VERSION_MINOR(mPhysicalDeviceInfo.DeviceId), VK_API_VERSION_PATCH(mPhysicalDeviceInfo.DeviceId));
		SPDLOG_INFO("\t Vendor ID      : {}.{}.{}", VK_API_VERSION_MAJOR(mPhysicalDeviceInfo.VendorId), VK_API_VERSION_MINOR(mPhysicalDeviceInfo.VendorId), VK_API_VERSION_PATCH(mPhysicalDeviceInfo.VendorId));
		

#pragma endregion

#pragma region Logical Device

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		uint32_t requestedQueueFamily = 0;
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			auto& props = queueFamilyProperties[i];
			bool hasGraphics = (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT;
			bool hasCompute = (props.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT;

			if (hasGraphics && hasCompute)
			{
				requestedQueueFamily = i;
				SPDLOG_INFO("Found queue family: {}", requestedQueueFamily);
			}
		}

		std::vector<const char*> logicalDeviceExtensions = {
			"VK_KHR_swapchain"
		};
		
		float priority = 1.f;
		VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.flags = 0;
		deviceQueueCreateInfo.queueFamilyIndex = requestedQueueFamily;
		deviceQueueCreateInfo.pQueuePriorities = &priority;
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pNext = nullptr;

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.enabledLayerCount; // Depreceated
		deviceCreateInfo.ppEnabledLayerNames; // Depreceated
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.enabledExtensionCount = logicalDeviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = logicalDeviceExtensions.data();
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
		deviceCreateInfo.pNext = nullptr;

		result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Vulkan logical device created");
		}
		else
		{
			SPDLOG_ERROR("Failed to create vulkan logical device");
			exit(1);
		}

		VkQueue queue = VK_NULL_HANDLE;
		vkGetDeviceQueue(mDevice, requestedQueueFamily, 0, &queue);

		mGraphicsQueue = MakeRef<GraphicsQueue>(mDevice, queue, requestedQueueFamily);
		mComputeQueue = MakeRef<GraphicsQueue>(mDevice, queue, requestedQueueFamily);
		mTransferQueue = MakeRef<GraphicsQueue>(mDevice, queue, requestedQueueFamily);

#pragma endregion

#pragma region Swapchain

		result = glfwCreateWindowSurface(mInstance, mWindow->GetGLFWWindow(), nullptr, &mSurface);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Created Window Surface");
		}
		else
		{
			SPDLOG_ERROR("Failed to create Window Surface");
			exit(1);
		}

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, surfaceFormats.data());

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, presentModes.data());

		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
			imageCount = surfaceCapabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo{};

		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.clipped = false;
		swapchainCreateInfo.compositeAlpha = (VkCompositeAlphaFlagBitsKHR)surfaceCapabilities.supportedCompositeAlpha;
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
		swapchainCreateInfo.imageColorSpace = surfaceFormats[0].colorSpace;
		swapchainCreateInfo.imageFormat = surfaceFormats[0].format;
		swapchainCreateInfo.imageSharingMode;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.oldSwapchain = nullptr;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		swapchainCreateInfo.surface = mSurface;

		result = vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapchain);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Vulkan swapchain Created");
		}
		else
		{
			SPDLOG_ERROR("Failed to create swapchain");
			exit(1);
		}

#pragma endregion

#pragma region Descriptor Pool

		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};

		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.maxSets = 10;
		descriptorPoolInfo.flags = 0;
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.pNext = nullptr;
		
		result = vkCreateDescriptorPool(mDevice, &descriptorPoolInfo, nullptr, &mDescriptorPool);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Descriptor pool created");
		}
		else
		{
			SPDLOG_ERROR("Failed to create descriptor pool");
			exit(1);
		}

#pragma endregion

	}

	// TODO
	GraphicsContext::~GraphicsContext()
	{
		vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
		vkDestroyDevice(mDevice, nullptr);
		vkDestroyInstance(mInstance, nullptr);
	}


	void GraphicsContext::BeginFrame()
	{
        
	}

	void GraphicsContext::EndFrame()
	{
        
	}
}
