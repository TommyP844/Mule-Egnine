#pragma once

#include "Ref.h"
#include "VulkanRenderTypes.h"
#include "WeakRef.h"
#include "VulkanTexture2D.h"

// Submodules
#include <glm/glm.hpp>

// STD
#include <vector>

namespace Mule
{
	class GraphicsContext;

	struct AttachmentDesc
	{
		TextureFormat Format = TextureFormat::NONE;
		TextureFlags Flags = TextureFlags::None;
	};

	struct FramebufferDescription
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t LayerCount = 1;
		std::vector<AttachmentDesc> Attachments;
		AttachmentDesc DepthAttachment;
	};

	class VulkanFrameBuffer
	{
	public:
		VulkanFrameBuffer(const FramebufferDescription& desc);
		~VulkanFrameBuffer();

		int GetWidth() const { return mDesc.Width; }
		int GetHeight() const { return mDesc.Height; }
		std::vector<VkClearValue> GetClearValues() const { return mClearValues; };
		uint32_t GetColorAttachmentCount() const { return mColorAttachments.size(); }
		WeakRef<VulkanTexture2D> GetColorAttachment(int index);
		WeakRef<VulkanTexture2D> GetDepthAttachment();

		void SetColorClearValue(int attachmentIndex, glm::vec4 clearColor);
		void SetColorClearValue(int attachmentIndex, glm::ivec4 clearColor);
		void SetColorClearValue(int attachmentIndex, glm::uvec4 clearColor);
		void SetDepthClearColor(float clearValue);

		void Resize(uint32_t width, uint32_t height);

	private:
		WeakRef<GraphicsContext> mContext;
		FramebufferDescription mDesc;

		std::vector<Ref<VulkanTexture2D>> mColorAttachments;
		Ref<VulkanTexture2D> mDepthAttachment;

		std::vector<VkClearValue> mClearValues;

		void Invalidate();
	};
}