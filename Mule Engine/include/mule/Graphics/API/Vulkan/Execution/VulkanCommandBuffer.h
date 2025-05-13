#pragma once

// Render Object
#include "Ref.h"
#include "WeakRef.h"

#include "Graphics/API/CommandBuffer.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VkCommandPool cmd);
		~VulkanCommandBuffer();
		
		void Reset() override;
		void Begin() override;
		void End() override;

		// Framebuffer
		void BeginSwapchainRendering() override;
		void EndSwapchainRendering() override;
		void BeginRendering(WeakRef<Framebuffer> framebuffer, WeakRef<GraphicsPipeline> shader, const std::vector<WeakRef<ShaderResourceGroup>>& groups = {}) override;
		void ClearFrameBuffer(WeakRef<Framebuffer> framebuffer) override;
		void EndRendering() override;

		// New API
		void BeginRendering(uint32_t width, uint32_t height, const std::vector<BeginRenderingAttachment>& colorAttachments, BeginRenderingAttachment depthAttachment) override;
		void BindPipeline(WeakRef<GraphicsPipeline> pipeline, const std::vector<WeakRef<ShaderResourceGroup>>& groups = {}) override;

		// WARNING, the following commands only work with 2d textures
		// Texture 2D
		void TranistionImageLayout(WeakRef<Texture> texture, ImageLayout newLayout) override;
		void CopyTexture(WeakRef<Texture> src, WeakRef<Texture> dst, const TextureCopyInfo& copyInfo) const override;
		void ReadTexture(WeakRef<Texture> texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, WeakRef<StagingBuffer> buffer) const override;

		// Graphics Pipelines
		void SetPushConstants(WeakRef<GraphicsPipeline> shader, ShaderStage stage, const void* data, uint32_t size) override;
		
		// Compute Pipelines
		void BindComputePipeline(WeakRef<ComputePipeline> shader, const std::vector<WeakRef<ShaderResourceGroup>>& groups = {}) override;
		void SetPushConstants(WeakRef<ComputePipeline> shader, void* data, uint32_t size) override;
		void Execute(uint32_t workGroupsX, uint32_t workGroupsY, uint32_t workGroupsZ) override;

		// Mesh
		void BindMesh(WeakRef<Mesh> mesh) override;
		void DrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount = 1) override;
		void BindAndDrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount) override;
		

		VkCommandBuffer GetHandle() const { return mCommandBuffer; }

	private:
		VkCommandPool mCommandPool;
		VkCommandBuffer mCommandBuffer;
	};
}
