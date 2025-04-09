#pragma once

#include "Ref.h"
#include "WeakRef.h"
#include "Application/Window.h"

#include "Graphics/Execution/GraphicsQueue.h"
#include "Graphics/Execution/Fence.h"
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Buffer/UniformBuffer.h"
#include "Graphics/RenderPass.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/SwapchainframeBuffer.h"
#include "Graphics/Shader/GraphicsShader.h"
#include "Graphics/Shader/ComputeShader.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/DescriptorSetLayout.h"
#include "Graphics/Mesh.h"
#include "Graphics/Buffer/StagingBuffer.h"

#include <Volk/volk.h>

// STD
#include <mutex>

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif // CreateSemaphore

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
		
		bool BeginFrame();
		void EndFrame(const std::vector<WeakRef<Semaphore>>& waitSemaphores = {});
		void ResizeSwapchain(uint32_t width, uint32_t height);

		Ref<DescriptorSet> CreateDescriptorSet(const std::vector<WeakRef<DescriptorSetLayout>>& layouts);
		Ref<UniformBuffer> CreateUniformBuffer(uint32_t bytes);
		Ref<Mesh> CreateMesh(const MeshDescription& description);
		Ref<GraphicsShader> CreateGraphicsShader(const fs::path& filepath);
		Ref<ComputeShader> CreateComputeShader(const fs::path& filepath);
		Ref<RenderPass> CreateRenderPass(const RenderPassDescription& renderPassDescription);
		Ref<Fence> CreateFence();
		Ref<Semaphore> CreateSemaphore();
		Ref<FrameBuffer> CreateFrameBuffer(const FramebufferDescription& frameBufferDesc);
		Ref<DescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<LayoutDescription>& layouts);
		Ref<StagingBuffer> CreateStagingBuffer(uint32_t size);


		WeakRef<GraphicsQueue> GetGraphicsQueue() const { return mGraphicsQueue; }
		WeakRef<GraphicsQueue> GetBackgroundGraphicsQueue() const { return mBackGroundQueue; }

		WeakRef<Window> GetWindow() const { return mWindow; }

		VkInstance GetInstance() const { return mInstance; }
		VkDevice GetDevice() const { return mDevice; }
		VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
		VkDescriptorPool GetDescriptorPool() const { return mDescriptorPool; }
		const PhysicalDeviceInfo& GetPhysicalDeviceInfo() const { return mPhysicalDeviceInfo; }
		Ref<SwapchainFrameBuffer> GetSwapchainFrameBuffer() const { return mFrameData[mFrameIndex].SwapchainFrameBuffer; }
		uint32_t GetMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return mMemoryProperties; }
		VkSampler GetLinearSampler() const { return mLinearSampler; }
		VkFormat GetSurfaceFormat() const { return mSurfaceFormat.format; }

	private:
		std::mutex mMutex;
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
		Ref<GraphicsQueue> mBackGroundQueue;

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
			Ref<Fence> ImageAcquiredFence;
			Ref<Semaphore> ImageAcquiredSemaphore;
			Ref<SwapchainFrameBuffer> SwapchainFrameBuffer;
			Ref<RenderPass> SwapchainRenderPass;
		};
		std::vector<FrameData> mFrameData;


		VkImageView CreateImageView(VkImage image, VkImageViewType viewtype, VkFormat format, int layers, int mips, bool depthImage);
		VulkanImage CreateImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageType imageType, int layers, int mips, VkImageUsageFlagBits usage);

	};
}