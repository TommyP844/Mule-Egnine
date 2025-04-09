#pragma once

// Render Object
#include "Ref.h"
#include "WeakRef.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderPass.h"
#include "Graphics/SwapchainframeBuffer.h"
#include "Graphics/Shader/GraphicsShader.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Shader/ComputeShader.h"
#include "Graphics/Buffer/StagingBuffer.h"

#include <Volk/volk.h>

namespace Mule
{
	class CommandBuffer
	{
	public:
		CommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
		~CommandBuffer();
		void Reset();
		void Begin();
		void End();

		void TransitionSwapchainFrameBufferForRendering(Ref<SwapchainFrameBuffer> fb);
		void TransitionSwapchainFrameBufferForPresent(Ref<SwapchainFrameBuffer> fb);

		// Framebuffers
		void BeginRenderPass(Ref<SwapchainFrameBuffer> framebuffer);
		void BeginRenderPass(WeakRef<FrameBuffer> framebuffer, WeakRef<GraphicsShader> shader, bool clearFramebuffer = false);
		void NextPass();
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
		void TranistionImageLayout(WeakRef<ITexture> texture, ImageLayout newLayout);
		void CopyTexture(WeakRef<ITexture> src, WeakRef<ITexture> dst, const TextureCopyInfo& copyInfo) const;
		void ReadTexture(WeakRef<ITexture> texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, WeakRef<StagingBuffer> buffer) const;

		// Shader
		void BindComputePipeline(WeakRef<ComputeShader> shader);
		void SetPushConstants(WeakRef<GraphicsShader> shader, ShaderStage stage, void* data, uint32_t size);
		void SetPushConstants(WeakRef<ComputeShader> shader, void* data, uint32_t size);
		void BindGraphicsDescriptorSet(WeakRef<GraphicsShader> shader, const std::vector<WeakRef<DescriptorSet>>& descriptorSets);
		void BindComputeDescriptorSet(WeakRef<ComputeShader> shader, WeakRef<DescriptorSet> descriptorSet);
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
