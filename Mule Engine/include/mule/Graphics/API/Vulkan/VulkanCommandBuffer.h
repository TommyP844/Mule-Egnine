#pragma once

// Render Object
#include "Ref.h"
#include "WeakRef.h"
#include "Graphics/Mesh.h"
#include "Graphics/SwapchainframeBuffer.h"
#include "VulkanGraphicsShader.h"
#include "VulkanFrameBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanComputeShader.h"
#include "VulkanStagingBuffer.h"

#include <Volk/volk.h>

namespace Mule
{
	class VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(VkCommandPool commandPool);
		~VulkanCommandBuffer();
		void Reset();
		void Begin();
		void End();

		void TransitionSwapchainFrameBufferForRendering(Ref<SwapchainFrameBuffer> fb);
		void TransitionSwapchainFrameBufferForPresent(Ref<SwapchainFrameBuffer> fb);

		// Framebuffers
		void BeginRenderPass(Ref<SwapchainFrameBuffer> framebuffer);
		void BeginRenderPass(WeakRef<VulkanFrameBuffer> framebuffer, WeakRef<VulkanGraphicsShader> shader);
		void ClearFrameBuffer(WeakRef<VulkanFrameBuffer> framebuffer);
		void EndRenderPass();

		struct TextureCopyInfo
		{
			glm::ivec3 SrcOffset = glm::ivec3(0);
			glm::ivec3 DstOffset = glm::ivec3(0);
			glm::ivec3 Extent;

			uint32_t SrcArrayLayer = 0;
			uint32_t SrcArrayLayerCount = 1;

			uint32_t DstArrayLayer = 0;
			uint32_t DstArrayLayerCount = 1;

			uint32_t SrcMipLevel = 0;
			uint32_t DstMipLevel = 0;
		};
		// Texture
		void TranistionImageLayout(WeakRef<IVulkanTexture> texture, ImageLayout newLayout);
		void CopyTexture(WeakRef<IVulkanTexture> src, WeakRef<IVulkanTexture> dst, const TextureCopyInfo& copyInfo) const;
		void ReadTexture(WeakRef<IVulkanTexture> texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, WeakRef<VulkanStagingBuffer> buffer) const;

		// Shader
		void BindComputePipeline(WeakRef<VulkanComputeShader> shader);
		void SetPushConstants(WeakRef<VulkanGraphicsShader> shader, ShaderStage stage, const void* data, uint32_t size);
		void SetPushConstants(WeakRef<VulkanComputeShader> shader, void* data, uint32_t size);
		void BindGraphicsDescriptorSet(WeakRef<VulkanGraphicsShader> shader, const std::vector<WeakRef<VulkanDescriptorSet>>& descriptorSets);
		void BindComputeDescriptorSet(WeakRef<VulkanComputeShader> shader, WeakRef<VulkanDescriptorSet> descriptorSet);
		void Execute(uint32_t workGroupsX, uint32_t workGroupsY, uint32_t workGroupsZ);

		// Mesh
		void BindMesh(WeakRef<Mesh> mesh);
		void DrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount = 1);
		void BindAndDrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount);
		

		VkCommandBuffer GetHandle() const { return mCommandBuffer; }

	private:
		VkDevice mDevice;
		VkCommandPool mCommandPool;
		VkCommandBuffer mCommandBuffer;
	};
}
