#include "Rendering/GraphicsContext.h"

// Submodules
#include "spdlog/spdlog.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"


// STD
#include <set>

namespace Mule
{
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		SPDLOG_INFO("{}\n", pCallbackData->pMessage);

		// Return VK_FALSE to let Vulkan continue
		return VK_FALSE;
	}

	GraphicsContext::GraphicsContext(const GraphicsContextDescription& description)
        :
        mWindow(description.Window),
		mInstance(VK_NULL_HANDLE),
		mDevice(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mSwapchain(VK_NULL_HANDLE),
		mFrameCount(2),
		mImageIndex(0),
		mFrameIndex(0)
	{

#pragma region Extensions
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
#pragma endregion

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

		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

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

		VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separateDepthStencilLayoutsFeatures{};
		separateDepthStencilLayoutsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES;

		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &separateDepthStencilLayoutsFeatures;

		vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &deviceFeatures2);

		// Enable the feature if supported
		if (separateDepthStencilLayoutsFeatures.separateDepthStencilLayouts) {
			separateDepthStencilLayoutsFeatures.separateDepthStencilLayouts = VK_TRUE;
		}

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
			"VK_KHR_swapchain",
			"VK_KHR_separate_depth_stencil_layouts",
			"VK_KHR_create_renderpass2"
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
		//deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures; // Not needed because of the line below
		deviceCreateInfo.pNext = &deviceFeatures2;

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
		uint32_t width = mWindow->GetWidth();
		uint32_t height = mWindow->GetHeight();

#pragma endregion

#pragma region Descriptor Pool

		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};

		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.maxSets = 1000 * poolSizes.size();
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

#pragma region Frame Data

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, surfaceFormats.data());
		mSurfaceFormat = surfaceFormats[0];

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, presentModes.data());

		// TODO: look into this it may cause issues
		mImageCount = 2;

		mCompositeAlphaFlags = (VkCompositeAlphaFlagBitsKHR)surfaceCapabilities.supportedCompositeAlpha;

		mFrameData.resize(2);

		RenderPassDescription renderPassDesc{};
		renderPassDesc.Attachments.push_back({ (TextureFormat)mSurfaceFormat.format });
		renderPassDesc.DepthAttachment = { TextureFormat::D32F };
		Ref<RenderPass> renderpass = CreateRenderPass(renderPassDesc);

		for (int i = 0; i < mFrameCount; i++)
		{
			mFrameData[i].ImageAcquiredFence = CreateFence();
			mFrameData[i].ImageAcquiredSemaphore = CreateSemaphore();

			mFrameData[i].SwapchainRenderPass = renderpass;
		}

		ResizeSwapchain(width, height);

#pragma endregion

	}

	GraphicsContext::~GraphicsContext()
	{
		mFrameData.clear();

		vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
		vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		vkDestroyDevice(mDevice, nullptr);
		
		auto destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroyDebugUtilsMessengerEXT)
		{
			destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
		}
		else {
			SPDLOG_ERROR("Failed to destroy debug messenger");
		}

		vkDestroyInstance(mInstance, nullptr);
	}

	void GraphicsContext::BeginFrame()
	{
		mFrameData[mImageIndex].ImageAcquiredFence->Wait();
		mFrameData[mFrameIndex].ImageAcquiredFence->Reset();

		mFrameData[mFrameIndex].ImageAcquiredSemaphore->Wait();

		VkResult result = vkAcquireNextImageKHR(
			mDevice,
			mSwapchain, 
			UINT64_MAX, 
			mFrameData[mFrameIndex].ImageAcquiredSemaphore->GetHandle(),
			mFrameData[mFrameIndex].ImageAcquiredFence->GetHandle(),
			&mImageIndex);
	}

	void GraphicsContext::EndFrame(std::vector<WeakRef<Semaphore>> gpuFences)
	{
		std::vector<VkSemaphore> waitSemaphores;
		for (const auto& semaphore : gpuFences)
		{
			waitSemaphores.emplace_back(semaphore->GetHandle());
		}

		waitSemaphores.push_back(mFrameData[mImageIndex].ImageAcquiredSemaphore->GetHandle());

		VkResult submitResult{};

		VkPresentInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.swapchainCount = 1;
		info.pSwapchains = &mSwapchain;
		info.waitSemaphoreCount = waitSemaphores.size();
		info.pWaitSemaphores = waitSemaphores.data();
		info.pImageIndices = &mImageIndex;
		info.pResults = &submitResult;
		info.pNext = nullptr;
		
		VkResult result = vkQueuePresentKHR(mGraphicsQueue->GetHandle(), &info);

		mFrameIndex ^= 1;
	}

	void GraphicsContext::ResizeSwapchain(uint32_t width, uint32_t height)
	{
		VkSwapchainCreateInfoKHR swapchainCreateInfo{};

		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.clipped = false;
		swapchainCreateInfo.compositeAlpha = mCompositeAlphaFlags;
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageExtent = { width, height };
		swapchainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;
		swapchainCreateInfo.imageFormat = mSurfaceFormat.format;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		swapchainCreateInfo.minImageCount = mImageCount;
		swapchainCreateInfo.oldSwapchain = mSwapchain;
		swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		swapchainCreateInfo.surface = mSurface;
		swapchainCreateInfo.pNext = nullptr;

		VkResult result = vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapchain);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Vulkan swapchain Created");
		}
		else
		{
			SPDLOG_ERROR("Failed to create swapchain");
			exit(1);
		}

		vkGetSwapchainImagesKHR(mDevice, mSwapchain, &mImageCount, nullptr);

		std::vector<VkImage> swapchainImages(mImageCount);
		vkGetSwapchainImagesKHR(mDevice, mSwapchain, &mImageCount, swapchainImages.data());

		// Create image views
		std::vector<VkImageView> swapchainImageViews(mImageCount);

		for (int i = 0; i < mFrameCount; i++)
		{
			SwapchainFrameBufferDescription swapchainFrameBufferDesc{};

			swapchainFrameBufferDesc.Width = width;
			swapchainFrameBufferDesc.Height = height;
			swapchainFrameBufferDesc.RenderPass = mFrameData[i].SwapchainRenderPass->GetHandle();

			swapchainFrameBufferDesc.ColorImage.Image = swapchainImages[i];
			swapchainFrameBufferDesc.ColorImage.ImageView = CreateImageView(
				swapchainFrameBufferDesc.ColorImage.Image,
				VK_IMAGE_VIEW_TYPE_2D, 
				mSurfaceFormat.format,
				1,
				1,
				false);

			swapchainFrameBufferDesc.DepthImage = CreateImage(
				width,
				height,
				1, 
				(VkFormat)TextureFormat::D32F,
				VK_IMAGE_TYPE_2D,
				1,
				1,
				(VkImageUsageFlagBits)(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));

			swapchainFrameBufferDesc.DepthImage.ImageView = CreateImageView(
				swapchainFrameBufferDesc.DepthImage.Image,
				VK_IMAGE_VIEW_TYPE_2D,
				(VkFormat)TextureFormat::D32F,
				1,
				1,
				true);

			swapchainFrameBufferDesc.Device = mDevice;


			mFrameData[i].SwapchainFrameBuffer = MakeRef<SwapchainFrameBuffer>(swapchainFrameBufferDesc);
		}
	}

	void GraphicsContext::WaitForDeviceIdle()
	{
		vkDeviceWaitIdle(mDevice);
	}

	Ref<RenderPass> GraphicsContext::CreateRenderPass(const RenderPassDescription& renderPassDescription)
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		std::vector<VkAttachmentReference> attachmentReferences;
		for (int i = 0; i < renderPassDescription.Attachments.size(); i++)
		{
			const Attachment& attachment = renderPassDescription.Attachments[i];

			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = (VkFormat)attachment.Format;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDesc.flags = 0;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

			attachmentDescriptions.push_back(attachmentDesc);

			VkAttachmentReference attachmentRef{};

			attachmentRef.attachment = i;
			attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachmentReferences.push_back(attachmentRef);
		}

		// Depth Attachment
		VkAttachmentReference depthAttachmentRef{};
		bool hasDepth = false;
		{
			if (renderPassDescription.DepthAttachment.Format != TextureFormat::NONE)
			{
				VkAttachmentDescription attachmentDesc{};
				attachmentDesc.format = (VkFormat)renderPassDescription.DepthAttachment.Format;
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				attachmentDesc.flags = 0;
				attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

				attachmentDescriptions.push_back(attachmentDesc);

				hasDepth = true;
				depthAttachmentRef.attachment = renderPassDescription.Attachments.size();
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			}

		}

		// Subpass
		VkSubpassDescription subpass{};
		{
			subpass.colorAttachmentCount = attachmentReferences.size();
			subpass.pColorAttachments = attachmentReferences.data();
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;
			subpass.pResolveAttachments = nullptr;
			subpass.flags = 0;
		}

		VkRenderPassCreateInfo renderPassCreateInfo{};

		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
		renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
		renderPassCreateInfo.flags = 0;
		renderPassCreateInfo.dependencyCount = 0;
		renderPassCreateInfo.pDependencies = nullptr;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;

		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkResult result = vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &renderPass);

		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Render Pass Created");
		}
		else
		{
			SPDLOG_ERROR("Failed to create render pass");
			return nullptr;
		}

		return MakeRef<RenderPass>(mDevice, renderPass);
	}

	Ref<Fence> GraphicsContext::CreateFence()
	{
		VkFenceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		info.pNext = nullptr;

		VkFence fence;
		VkResult result = vkCreateFence(mDevice, &info, nullptr, &fence);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Fence Created");
		}
		else
		{
			SPDLOG_ERROR("Failed to create Fence");
		}

		return MakeRef<Fence>(mDevice, fence);
	}

	Ref<Semaphore> GraphicsContext::CreateSemaphore()
	{
		VkSemaphoreCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;

		VkSemaphore semaphore;
		VkResult result = vkCreateSemaphore(mDevice, &info, nullptr, &semaphore);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Semaphore Created");
		}
		else
		{
			SPDLOG_ERROR("Failed to create Semaphore");
		}

		return MakeRef<Semaphore>(mDevice, semaphore);
	}

	Ref<FrameBuffer> GraphicsContext::CreateFrameBuffer(const FramebufferDescription& frameBufferDesc)
	{
	
		VkFramebufferCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = frameBufferDesc.Width;
		info.height = frameBufferDesc.Height;
		info.layers = frameBufferDesc.Layers;
		info.renderPass = frameBufferDesc.RenderPass->GetHandle();
		info.flags = 0;
		info.attachmentCount = 0;
		info.pAttachments = nullptr;
		info.pNext = nullptr;

		VkFramebuffer frameBuffer = VK_NULL_HANDLE;
		vkCreateFramebuffer(mDevice, &info, nullptr, &frameBuffer);


		return nullptr;
	}

	VkImageView GraphicsContext::CreateImageView(VkImage image, VkImageViewType viewtype, VkFormat format, int layers, int mips, bool depthImage)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = image;
		viewCreateInfo.viewType = viewtype;
		viewCreateInfo.format = format; // Provided during swapchain creation
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = depthImage ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = mips;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = layers;

		VkImageView imageView = VK_NULL_HANDLE;
		VkResult result = vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &imageView);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create image view");
		}

		return imageView;
	}

	VulkanImage GraphicsContext::CreateImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageType imageType, int layers, int mips, VkImageUsageFlagBits usage)
	{
		VulkanImage vulkanImage;

		VkImageCreateInfo info{};

		std::set<uint32_t> queueFamilyIndicexSet = {
			mGraphicsQueue->GetQueueFamilyIndex(),
			mComputeQueue->GetQueueFamilyIndex(),
			mTransferQueue->GetQueueFamilyIndex()
		};

		std::vector<uint32_t> queueFamilyIndices;
		std::copy(queueFamilyIndicexSet.begin(), queueFamilyIndicexSet.end(), std::back_inserter(queueFamilyIndices));

		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags;
		info.imageType = imageType;
		info.format = format;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = depth;
		info.mipLevels = mips;
		info.arrayLayers = layers;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usage;
		info.sharingMode = queueFamilyIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = queueFamilyIndices.size();
		info.pQueueFamilyIndices = queueFamilyIndices.data();
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(mDevice, &info, nullptr, &vulkanImage.Image);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to create image");
		}
		

		uint32_t size = width * height * depth * layers * GetFormatSize((TextureFormat)format);

		VkMemoryRequirements requierments;
		vkGetImageMemoryRequirements(mDevice, vulkanImage.Image, &requierments);

		uint32_t memoryTypeIndex = 0;
		for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; i++) {
			// Check if the memory type is supported
			if ((requierments.memoryTypeBits & (1 << i)) &&
				(mMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
				memoryTypeIndex = i;
			}
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = requierments.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		allocInfo.pNext = nullptr;

		result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &vulkanImage.Memory);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to allocate device memory");
		}

		result = vkBindImageMemory(mDevice, vulkanImage.Image, vulkanImage.Memory, 0);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to bind image memory");
		}
		
		return vulkanImage;
	}

}
