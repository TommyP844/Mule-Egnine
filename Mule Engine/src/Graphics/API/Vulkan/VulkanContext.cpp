
#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/Buffer/IVulkanBuffer.h"
#include "Graphics/API/Vulkan/Execution/VulkanCommandPool.h"

#include <spdlog/spdlog.h>

#include <Volk/volk.c>

namespace Mule::Vulkan
{
	VulkanContext* VulkanContext::mVulkanContext = nullptr;

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		SPDLOG_INFO("{}\n", pCallbackData->pMessage);

		// Return VK_FALSE to let Vulkan continue
		return VK_FALSE;
	}

	void VulkanContext::Init(Ref<Window> window)
	{
		mVulkanContext = new VulkanContext(window);
		mVulkanContext->InitInternal();
	}

	void VulkanContext::Shutdown()
	{
		delete mVulkanContext;
	}

	VulkanContext& VulkanContext::Get()
	{
		return *mVulkanContext;
	}

	VulkanContext::VulkanContext(Ref<Window> window)
		:
		mWindow(window),
		mInstance(VK_NULL_HANDLE),
		mDevice(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mSwapchain(VK_NULL_HANDLE),
		mFrameCount(2),
		mFrameIndex(0)
	{
	}

	VulkanContext::~VulkanContext()
	{
		vkDeviceWaitIdle(mDevice);

		for (auto& frameData : mFrameData)
		{
			vkDestroyImageView(mDevice, frameData.ColorImageView, nullptr);
			vkDestroyImageView(mDevice, frameData.DepthImageView, nullptr);

			vkFreeMemory(mDevice, frameData.DepthImageMemory, nullptr);

			vkDestroyImage(mDevice, frameData.DepthImage, nullptr);

			frameData.ImageAcquiredFence = nullptr;
			frameData.ImageAcquiredSemaphore = nullptr;
		}

		for (auto [tid, commandPool] : mContextCommandPools)
		{
			vkDestroyCommandPool(mDevice, commandPool, nullptr);
		}

		vkDestroySampler(mDevice, mLinearSampler, nullptr);

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

	void VulkanContext::InitInternal()
	{

#pragma region Extensions
		std::vector<const char*> enabledLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> enabledExtensions = {
			"VK_EXT_debug_utils",
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		};

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (int i = 0; i < glfwExtensionCount; i++)
		{
			enabledExtensions.push_back(glfwExtensions[i]);
		}
#pragma endregion

#pragma region Instance


		VkResult result = volkInitialize();
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Volk failed to init, Code: {}", (uint32_t)result);
			exit(1);
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_3;
		//appInfo.applicationVersion = description.AppVersion;
		//appInfo.engineVersion = description.EngineVersion;
		//appInfo.pApplicationName = description.AppName.c_str();
		//appInfo.pEngineName = description.EngineName.c_str();
		appInfo.pNext = nullptr;

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.enabledExtensionCount = enabledExtensions.size();
		instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
		instanceInfo.enabledLayerCount = enabledLayers.size();
		instanceInfo.ppEnabledLayerNames = enabledLayers.data();
		instanceInfo.flags = 0;
		instanceInfo.pApplicationInfo = nullptr;

		result = vkCreateInstance(&instanceInfo, nullptr, &mInstance);
		if (result == VK_SUCCESS)
		{
			SPDLOG_INFO("Vulkan initialized");
		}
		else
		{
			SPDLOG_ERROR("Vulkan failed to init, Code: {}", (uint32_t)result);
			exit(1);
		}

		volkLoadInstance(mInstance);

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

		auto createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
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

#pragma endregion

#pragma region Logical Device

		VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features{};
		sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
		sync2Features.synchronization2 = VK_TRUE;
		sync2Features.pNext = nullptr;

		VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures{};
		timelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
		timelineFeatures.timelineSemaphore = VK_TRUE;
		timelineFeatures.pNext = &sync2Features;

		VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamicRenderingUnusedAttachmentsFeatures{};
		dynamicRenderingUnusedAttachmentsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT;
		dynamicRenderingUnusedAttachmentsFeatures.dynamicRenderingUnusedAttachments = VK_TRUE;
		dynamicRenderingUnusedAttachmentsFeatures.pNext = &timelineFeatures;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
		dynamicRenderingFeatures.pNext = &dynamicRenderingUnusedAttachmentsFeatures;

		// Enable descriptor indexing features
		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		indexingFeatures.runtimeDescriptorArray = VK_TRUE;
		indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
		indexingFeatures.pNext = &dynamicRenderingFeatures;

		// Enable separate depth-stencil layouts
		VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separateDepthStencilLayoutsFeatures{};
		separateDepthStencilLayoutsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES;

		// Chain features together
		separateDepthStencilLayoutsFeatures.pNext = &indexingFeatures;

		// Query features support
		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &separateDepthStencilLayoutsFeatures;

		vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &deviceFeatures2);

		// Enable supported features
		if (separateDepthStencilLayoutsFeatures.separateDepthStencilLayouts) {
			separateDepthStencilLayoutsFeatures.separateDepthStencilLayouts = VK_TRUE;
		}

		std::vector<VkQueueFamilyProperties> properties;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
		properties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, properties.data());

		VkDeviceQueueCreateInfo queueCreateInfo;
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			auto& familyInfo = properties[i];

			bool hasGraphics = familyInfo.queueFlags & VK_QUEUE_GRAPHICS_BIT;
			bool hasCompute = familyInfo.queueFlags & VK_QUEUE_COMPUTE_BIT;

			if (!hasGraphics || !hasCompute)
				continue;

			mQueueFamilyIndex = i;
			mQueueFamilyProperties = familyInfo;

			float* priorities = new float[familyInfo.queueCount];
			for (int j = 0; j < familyInfo.queueCount; j++)
			{
				priorities[j] = 1.f;
				mFreeQueueIndices.push(j);
			}


			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = i;
			queueCreateInfo.pQueuePriorities = priorities;
			queueCreateInfo.queueCount = familyInfo.queueCount;
			queueCreateInfo.pNext = nullptr;
		}


		std::vector<const char*> logicalDeviceExtensions = {
			"VK_KHR_swapchain",
			"VK_KHR_separate_depth_stencil_layouts",
			"VK_KHR_create_renderpass2",
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_MAINTENANCE1_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			"VK_KHR_depth_stencil_resolve",
			"VK_EXT_dynamic_rendering_unused_attachments",
			"VK_KHR_timeline_semaphore",
			"VK_KHR_synchronization2",
		};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.enabledLayerCount; // Depreceated
		deviceCreateInfo.ppEnabledLayerNames; // Depreceated
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.enabledExtensionCount = logicalDeviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = logicalDeviceExtensions.data();
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
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

		// We need to release the dynamically allocated priorities for each queue
		delete[] queueCreateInfo.pQueuePriorities;

		volkLoadDevice(mDevice);

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

		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &capabilities);
		if (capabilities.minImageCount > 2)
		{
			mFrameCount = capabilities.minImageCount;
		}
		else
		{
			mFrameCount = 2;
		}

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
		descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
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

#pragma region Sampler

		VkSamplerCreateInfo samplerCreateInfo{};

		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.flags = 0;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.f;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.maxAnisotropy = 8.f;
		samplerCreateInfo.compareEnable = VK_FALSE;
		//samplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;
		samplerCreateInfo.minLod = 0.f;
		samplerCreateInfo.maxLod = 1.f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_MAX_ENUM;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		result = vkCreateSampler(mDevice, &samplerCreateInfo, nullptr, &mLinearSampler);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create linear sampler");
		}

#pragma endregion

#pragma region Context Queue

		mContextQueue = CreateQueue();

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

		mCompositeAlphaFlags = (VkCompositeAlphaFlagBitsKHR)surfaceCapabilities.supportedCompositeAlpha;

		mFrameData.resize(mFrameCount);

		for (int i = 0; i < mFrameCount; i++)
		{
			mFrameData[i].ImageAcquiredFence = MakeRef<VulkanFence>();
			mFrameData[i].ImageAcquiredFence->Wait();
			mFrameData[i].ImageAcquiredFence->Reset();
			mFrameData[i].ImageAcquiredSemaphore = MakeRef<VulkanSemaphore>();
		}

		ResizeSwapchain(width, height);

#pragma endregion

	}

	Ref<VulkanCommandBuffer> VulkanContext::BeginSingleTimeCommandBuffer()
	{
		std::thread::id tid = std::this_thread::get_id();

		VkCommandPool pool = VK_NULL_HANDLE;
		{
			std::lock_guard<std::mutex> lock(mCommandPoolMutex);
			auto iter = mContextCommandPools.find(tid);
			if (iter != mContextCommandPools.end())
			{
				pool = iter->second;
			}
			else
			{
				VkCommandPoolCreateInfo createInfo{
					.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
					.pNext = nullptr,
					.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
					.queueFamilyIndex = mQueueFamilyIndex
				};

				vkCreateCommandPool(mDevice, &createInfo, nullptr, &pool);

				mContextCommandPools[tid] = pool;
			}
		}

		Ref<VulkanCommandBuffer> cmd = MakeRef<VulkanCommandBuffer>(pool);

		cmd->Begin();

		return cmd;
	}

	void VulkanContext::EndSingleTimeCommandBuffer(Ref<VulkanCommandBuffer> cmd)
	{
		cmd->End();

		VkFence fence = VK_NULL_HANDLE;

		VkFenceCreateInfo fenceInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};

		VkCommandBuffer vkCmd = cmd->GetHandle();

		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = nullptr,
			.commandBufferCount = 1,
			.pCommandBuffers = &vkCmd,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores = nullptr
		};

		vkCreateFence(mDevice, &fenceInfo, nullptr, &fence);

		{
			std::lock_guard<std::mutex> lock(mQueueMutex);
			vkQueueSubmit(mContextQueue, 1, &submitInfo, fence);
		}
		
		vkWaitForFences(mDevice, 1, &fence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(mDevice, fence, nullptr);
	}

	void VulkanContext::CreateMemoryBarrier(VkImageMemoryBarrier& barrier, WeakRef<IVulkanTexture> texture, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = GetAccessFlagsFromLayout(oldLayout);
		barrier.dstAccessMask = GetAccessFlagsFromLayout(newLayout);
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = mQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = mQueueFamilyIndex;
		barrier.image = texture->GetVulkanImage().Image;
		barrier.subresourceRange.aspectMask = texture->GetImageAspect();
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = levelCount;
		barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
		barrier.subresourceRange.layerCount = layerCount;
	}

	VkAccessFlags VulkanContext::GetAccessFlagsFromLayout(VkImageLayout layout)
	{
		switch (layout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_SHADER_READ_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		default:
			return 0;
		}
	}

	VkPipelineStageFlags VulkanContext::GetPipelineStageFlagFromLayout(VkImageLayout layout)
	{
		switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// No previous usage; typically paired with srcAccessMask = 0
			return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		case VK_IMAGE_LAYOUT_GENERAL:
			// Generic access; shader or compute usage
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return VK_PIPELINE_STAGE_HOST_BIT;

		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			// Presentation engine usage
			return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

		default:
			// Fallback, use most conservative option
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		}
	}

	bool VulkanContext::BeginFrame()
	{
		uint32_t frameIndex = mFrameIndex;

		VkResult result = vkAcquireNextImageKHR(
			mDevice,
			mSwapchain,
			UINT64_MAX,
			VK_NULL_HANDLE,
			mFrameData[frameIndex].ImageAcquiredFence->GetHandle(),
			&mFrameIndex);		

		mFrameData[frameIndex].ImageAcquiredFence->Wait();
		mFrameData[frameIndex].ImageAcquiredFence->Reset();

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			int width, height;
			glfwGetFramebufferSize(mWindow->GetGLFWWindow(), &width, &height);
			ResizeSwapchain(width, height);
			return false;
		}
		else if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to get next image from swap chain");
			return false;
		}

		return true;
	}

	void VulkanContext::EndFrame(const std::vector<VkSemaphore>& waitSemaphores)
	{
		VkResult results;

		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
			.pWaitSemaphores = waitSemaphores.data(),
			.swapchainCount = 1,
			.pSwapchains = &mSwapchain,
			.pImageIndices = &mFrameIndex,
			.pResults = &results,
		};

		{
			std::lock_guard<std::mutex> lock(mQueueMutex);
			vkQueuePresentKHR(mContextQueue, &presentInfo);
		}

		mFrameIndex ^= 1;
	}

	void VulkanContext::ResizeSwapchain(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0) return;
		vkDeviceWaitIdle(mDevice);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities);

		vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

		VkSwapchainCreateInfoKHR swapchainCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = mSurface,
			.minImageCount = mFrameCount,
			.imageFormat = mSurfaceFormat.format,
			.imageColorSpace = mSurfaceFormat.colorSpace,
			.imageExtent = surfaceCapabilities.currentExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			.compositeAlpha = mCompositeAlphaFlags,
			.presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
			.clipped = false,
			.oldSwapchain = VK_NULL_HANDLE,
		};

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

		std::vector<VkImage> colorImages(mFrameCount);
		vkGetSwapchainImagesKHR(mDevice, mSwapchain, &mFrameCount, colorImages.data());
		
		auto cmd = BeginSingleTimeCommandBuffer();

		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			if (mFrameData[i].ColorImageView != VK_NULL_HANDLE)
				vkDestroyImageView(mDevice, mFrameData[i].ColorImageView, nullptr);
			if (mFrameData[i].DepthImageView != VK_NULL_HANDLE)
				vkDestroyImageView(mDevice, mFrameData[i].DepthImageView, nullptr);
			if (mFrameData[i].DepthImageMemory != VK_NULL_HANDLE)
				vkFreeMemory(mDevice, mFrameData[i].DepthImageMemory, nullptr);
			if (mFrameData[i].DepthImage != VK_NULL_HANDLE)
				vkDestroyImage(mDevice, mFrameData[i].DepthImage, nullptr);

			mFrameData[i].ColorImage = colorImages[i];

			VkImageViewCreateInfo colorViewInfo{};
			colorViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorViewInfo.pNext = nullptr;
			colorViewInfo.flags = 0;
			colorViewInfo.image = colorImages[i];
			colorViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorViewInfo.format = mSurfaceFormat.format;
			colorViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			colorViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			colorViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			colorViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			colorViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorViewInfo.subresourceRange.baseArrayLayer = 0;
			colorViewInfo.subresourceRange.layerCount = 1;
			colorViewInfo.subresourceRange.baseMipLevel = 0;
			colorViewInfo.subresourceRange.levelCount = 1;

			vkCreateImageView(mDevice, &colorViewInfo, nullptr, &mFrameData[i].ColorImageView);

			VkImageCreateInfo depthImageInfo{};
			depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			depthImageInfo.pNext = nullptr;
			depthImageInfo.flags = 0;
			depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
			depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
			depthImageInfo.extent = { surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height, 1 };
			depthImageInfo.mipLevels = 1;
			depthImageInfo.arrayLayers = 1;
			depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			depthImageInfo.queueFamilyIndexCount = 1;
			depthImageInfo.pQueueFamilyIndices = &mQueueFamilyIndex;
			depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			vkCreateImage(mDevice, &depthImageInfo, nullptr, &mFrameData[i].DepthImage);

			VkMemoryRequirements requierments;
			vkGetImageMemoryRequirements(mDevice, mFrameData[i].DepthImage, &requierments);

			VkMemoryAllocateInfo depthAllocInfo{};
			depthAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			depthAllocInfo.pNext = nullptr;
			depthAllocInfo.memoryTypeIndex = GetMemoryTypeIndex(requierments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);;
			depthAllocInfo.allocationSize = requierments.size;			

			vkAllocateMemory(mDevice, &depthAllocInfo, nullptr, &mFrameData[i].DepthImageMemory);

			vkBindImageMemory(mDevice, mFrameData[i].DepthImage, mFrameData[i].DepthImageMemory, 0);

			VkImageViewCreateInfo depthViewInfo{};
			depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			depthViewInfo.pNext = nullptr;
			depthViewInfo.flags = 0;
			depthViewInfo.image = mFrameData[i].DepthImage;
			depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			depthViewInfo.format = VK_FORMAT_D32_SFLOAT;
			depthViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			depthViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			depthViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			depthViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			depthViewInfo.subresourceRange.baseArrayLayer = 0;
			depthViewInfo.subresourceRange.layerCount = 1;
			depthViewInfo.subresourceRange.baseMipLevel = 0;
			depthViewInfo.subresourceRange.levelCount = 1;

			vkCreateImageView(mDevice, &depthViewInfo, nullptr, &mFrameData[i].DepthImageView);

			// Layout transitions
			VkImageMemoryBarrier toColorAttachment{};
			toColorAttachment.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			toColorAttachment.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			toColorAttachment.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			toColorAttachment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			toColorAttachment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			toColorAttachment.image = mFrameData[i].ColorImage;
			toColorAttachment.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			toColorAttachment.subresourceRange.baseMipLevel = 0;
			toColorAttachment.subresourceRange.levelCount = 1;
			toColorAttachment.subresourceRange.baseArrayLayer = 0;
			toColorAttachment.subresourceRange.layerCount = 1;
			toColorAttachment.srcAccessMask = 0;
			toColorAttachment.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			vkCmdPipelineBarrier(
				cmd->GetHandle(),
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &toColorAttachment
			);

			// Transition to PRESENT_SRC_KHR
			VkImageMemoryBarrier toPresent{};
			toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			toPresent.image = mFrameData[i].ColorImage;
			toPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			toPresent.subresourceRange.baseMipLevel = 0;
			toPresent.subresourceRange.levelCount = 1;
			toPresent.subresourceRange.baseArrayLayer = 0;
			toPresent.subresourceRange.layerCount = 1;
			toPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			toPresent.dstAccessMask = 0;

			vkCmdPipelineBarrier(
				cmd->GetHandle(),
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &toPresent
			);

			VkImageMemoryBarrier depthBarrier{};
			depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depthBarrier.image = mFrameData[i].DepthImage;
			depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			depthBarrier.subresourceRange.baseMipLevel = 0;
			depthBarrier.subresourceRange.levelCount = 1;
			depthBarrier.subresourceRange.baseArrayLayer = 0;
			depthBarrier.subresourceRange.layerCount = 1;
			depthBarrier.srcAccessMask = 0;
			depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			vkCmdPipelineBarrier(
				cmd->GetHandle(),
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &depthBarrier
			);
		}

		EndSingleTimeCommandBuffer(cmd);

	}

	void VulkanContext::AwaitIdle()
	{
		vkDeviceWaitIdle(mDevice);
	}

	WeakRef<Window> VulkanContext::GetWindow() const
	{
		return mWindow;
	}

	VkPhysicalDevice VulkanContext::GetPhysicalDevice()
	{
		return mPhysicalDevice;
	}

	uint32_t VulkanContext::GetMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (mMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		SPDLOG_ERROR("Failed to find memory type index");
		return 0;
	}

	void VulkanContext::CopyBuffer(Ref<VulkanCommandBuffer> cmd, WeakRef<IVulkanBuffer> src, WeakRef<IVulkanBuffer> dst)
	{
		assert(src->GetSize() == dst->GetSize() && "src & dst buffers must be the same size");

		VkBufferCopy region{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = src->GetSize()
		};
		
		vkCmdCopyBuffer(
			cmd->GetHandle(),
			src->GetBuffer(),
			dst->GetBuffer(),
			1,
			&region);
	}

	void VulkanContext::CopyBufferToImage(Ref<VulkanCommandBuffer> cmd, VkBuffer buffer, VkImage image, const VkBufferImageCopy& bufferImageCopy)
	{
		vkCmdCopyBufferToImage(
			cmd->GetHandle(),
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferImageCopy);
	}

	void VulkanContext::CopyImageToBuffer(Ref<VulkanCommandBuffer> cmd, VkImage image, VkBuffer buffer, const VkBufferImageCopy& bufferImageCopy)
	{
		vkCmdCopyImageToBuffer(
			cmd->GetHandle(),
			image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			buffer,
			1,
			&bufferImageCopy);
	}

	void VulkanContext::BlitMip(Ref<VulkanCommandBuffer> cmd, WeakRef<IVulkanTexture> texture, const VkImageBlit& blit)
	{
		VkImage image = texture->GetVulkanImage().Image;

		vkCmdBlitImage(
			cmd->GetHandle(),
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, 
			&blit,
			VK_FILTER_LINEAR);
	}

	void VulkanContext::TransitionImageLayout(Ref<VulkanCommandBuffer> cmd, WeakRef<IVulkanTexture> texture, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		VkImageMemoryBarrier barrier;
		CreateMemoryBarrier(barrier, texture, oldLayout, newLayout, baseMipLevel, levelCount, baseArrayLayer, layerCount);

		vkCmdPipelineBarrier(
			cmd->GetHandle(),
			GetPipelineStageFlagFromLayout(oldLayout),
			GetPipelineStageFlagFromLayout(newLayout),
			0, 
			0,
			nullptr,
			0,
			nullptr, 
			1,
			&barrier);
	}

	VkImageView VulkanContext::GetCurrentSwapchainColorImageView()
	{
		return mFrameData[mFrameIndex].ColorImageView;
	}

	VkImageView VulkanContext::GetCurrentSwapchainDepthImageView()
	{
		return mFrameData[mFrameIndex].DepthImageView;
	}

	VkImage VulkanContext::GetCurrentSwapchainColorImage()
	{
		return mFrameData[mFrameIndex].ColorImage;
	}

	uint32_t VulkanContext::GetQueueFamilyIndex() const
	{
		return mQueueFamilyIndex;
	}

	VkQueue VulkanContext::CreateQueue()
	{
		assert(!mFreeQueueIndices.empty() && "Vulkan device out of queues");

		uint32_t queueIndex = mFreeQueueIndices.top();
		mFreeQueueIndices.pop();

		VkQueue queue;
		vkGetDeviceQueue(mDevice, mQueueFamilyIndex, queueIndex, &queue);

		mAllocatedQueueIndices[queue] = queueIndex;

		return queue;
	}

	void VulkanContext::ReleaseQueue(VkQueue queue)
	{
		assert(mAllocatedQueueIndices.find(queue) != mAllocatedQueueIndices.end() && "Trying to free non-existent vulkan queue");

		uint32_t queueIndex = mAllocatedQueueIndices[queue];
		mFreeQueueIndices.push(queueIndex);
		mAllocatedQueueIndices.erase(queue);
	}
}