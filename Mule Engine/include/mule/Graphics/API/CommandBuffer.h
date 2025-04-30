#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/API/Texture.h"
#include "Graphics/API/Framebuffer.h"
#include "Graphics/API/GraphicsPipeline.h"
#include "Graphics/API/StagingBuffer.h"
#include "Graphics/API/ShaderResourceGroup.h"

namespace Mule
{
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

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Reset() = 0; 
		virtual void Begin() = 0;
		virtual void End() = 0;


		// Framebuffer
		virtual void BeginSwapchainRendering() = 0;
		virtual void EndSwapchainRendering() = 0;
		virtual void BeginRendering(WeakRef<Framebuffer> framebuffer, WeakRef<GraphicsPipeline> shader) = 0;
		virtual void ClearFrameBuffer(WeakRef<Framebuffer> framebuffer) = 0;
		virtual void EndRendering() = 0;

		
		// Texture
		virtual void TranistionImageLayout(WeakRef<Texture> texture, ImageLayout newLayout) = 0;
		virtual void CopyTexture(WeakRef<Texture> src, WeakRef<Texture> dst, const TextureCopyInfo& copyInfo) const = 0;
		virtual void ReadTexture(WeakRef<Texture> texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, WeakRef<StagingBuffer> buffer) const = 0;

		// Shader
		virtual void BindPipeline(WeakRef<GraphicsPipeline> pipeline, const std::vector<WeakRef<ShaderResourceGroup>>& groups = {}) = 0;
		virtual void SetPushConstants(WeakRef<GraphicsPipeline> shader, ShaderStage stage, const void* data, uint32_t size) = 0;
		virtual void BindGraphicsDescriptorSet(WeakRef<GraphicsPipeline> shader, const std::vector<WeakRef<ShaderResourceGroup>>& descriptorSets) = 0;
		virtual void Execute(uint32_t workGroupsX, uint32_t workGroupsY, uint32_t workGroupsZ) = 0;
		//virtual void SetPushConstants(WeakRef<VulkanComputeShader> shader, void* data, uint32_t size);
		//virtual void BindComputeDescriptorSet(WeakRef<VulkanComputeShader> shader, WeakRef<VulkanDescriptorSet> descriptorSet);
		//virtual void BindComputePipeline(WeakRef<VulkanComputeShader> shader);
		virtual void BlendEnable(uint32_t attachment, bool blendEnable) = 0;

		// Mesh
		virtual void BindMesh(WeakRef<Mesh> mesh) = 0;
		virtual void DrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount = 1) = 0;
		virtual void BindAndDrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount) = 0;

	protected:
		CommandBuffer() = default;
	};
}
