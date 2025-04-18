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

	class FrameBuffer
	{
	public:
		FrameBuffer(WeakRef<GraphicsContext> context, const FramebufferDescription& desc);
		~FrameBuffer();

		int GetWidth() const { return mDesc.Width; }
		int GetHeight() const { return mDesc.Height; }
		std::vector<VkClearValue> GetClearValues() const { return mClearValues; };
		uint32_t GetColorAttachmentCount() const { return mColorAttachments.size(); }
		WeakRef<Texture2D> GetColorAttachment(int index);
		WeakRef<Texture2D> GetDepthAttachment();

		void SetColorClearValue(int attachmentIndex, glm::vec4 clearColor);
		void SetColorClearValue(int attachmentIndex, glm::ivec4 clearColor);
		void SetColorClearValue(int attachmentIndex, glm::uvec4 clearColor);
		void SetDepthClearColor(float clearValue);

		void Resize(uint32_t width, uint32_t height);

	private:
		WeakRef<GraphicsContext> mContext;
		FramebufferDescription mDesc;

		std::vector<Ref<Texture2D>> mColorAttachments;
		Ref<Texture2D> mDepthAttachment;

		std::vector<VkClearValue> mClearValues;

		void Invalidate();
	};
}