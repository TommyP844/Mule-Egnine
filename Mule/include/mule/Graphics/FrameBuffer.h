#pragma once

#include "Ref.h"
#include "RenderTypes.h"
#include "RenderPass.h"
#include "WeakRef.h"
#include "Texture/Texture2D.h"

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
	};

	struct FramebufferDescription
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t LayerCount = 1;
		Ref<RenderPass> RenderPass = nullptr;
		std::vector<AttachmentDesc> Attachments;
		AttachmentDesc DepthAttachment;
	};

	class FrameBuffer
	{
	public:
		FrameBuffer(WeakRef<GraphicsContext> context, const FramebufferDescription& desc);
		~FrameBuffer();

		VkFramebuffer GetHandle() const { return mFrameBuffer; }
		int GetWidth() const { return mDesc.Width; }
		int GetHeight() const { return mDesc.Height; }
		std::vector<VkClearValue> GetClearValues() const { return mClearValues; };
		WeakRef<Texture2D> GetColorAttachment(int index);
		WeakRef<Texture2D> GetDepthAttachment();

		void SetColorClearValue(int attachmentIndex, glm::vec4 clearColor);
		void SetColorClearValue(int attachmentIndex, glm::ivec4 clearColor);
		void SetColorClearValue(int attachmentIndex, glm::uvec4 clearColor);
		void SetDepthClearColor(float clearValue);

		void Resize(uint32_t width, uint32_t height);

	private:
		WeakRef<GraphicsContext> mContext;
		VkFramebuffer mFrameBuffer;
		FramebufferDescription mDesc;

		std::vector<Ref<Texture2D>> mColorAttachments;
		Ref<Texture2D> mDepthAttachment;

		std::vector<VkClearValue> mClearValues;

		void Invalidate();
	};
}