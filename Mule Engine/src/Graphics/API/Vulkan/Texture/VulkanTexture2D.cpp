#include "Graphics/API/Vulkan/Texture/VulkanTexture2D.h"

#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/VulkanTypeConversion.h"
#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	VulkanTexture2D::VulkanTexture2D(const std::string& name, const Buffer& buffer, int width, int height, TextureFormat format, TextureFlags flags)
		:
		Texture2D(name, format, flags)
	{
		mWidth = width;
		mHeight = height;
		mDepth = 1;
		mArrayLayers = 1;

		VulkanContext& context = VulkanContext::Get();

		if ((TextureFlags::GenerateMips & flags) == TextureFlags::GenerateMips)
		{
			mMipLevels = glm::log2(glm::max<float>(mWidth, mHeight));
			mHasMips = true;
		}
		else
		{
			mMipLevels = 1;
			mHasMips = false;
		}
				
		VkFormat textureFormat = GetVulkanFormat(format);
		VkImageAspectFlags aspectFlags = ((flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget 
			&& (flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment)
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget) 
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if ((flags & TextureFlags::StorageImage) == TextureFlags::StorageImage)	usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

		bool success = Init(
			VK_IMAGE_TYPE_2D,
			textureFormat,
			mWidth,
			mHeight,
			1,
			mMipLevels,
			1,
			usageFlags,
			aspectFlags,
			VK_IMAGE_VIEW_TYPE_2D
		);

		auto cmd = context.BeginSingleTimeCommandBuffer();
		
		// We need this to stay alive untul the constructor leaves
		Ref<VulkanStagingBuffer> stagingBuffer;

		if (buffer)
		{
			// Transition every mip level to be transfer source
			context.TransitionImageLayout(
				cmd,
				this,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				0,
				mMipLevels,
				0,
				1);

			stagingBuffer = MakeRef<VulkanStagingBuffer>(buffer);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = aspectFlags;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { mWidth, mHeight, 1 };

			VkBuffer buf = stagingBuffer->GetBuffer();
			VkImage image = GetVulkanImage().Image;

			context.CopyBufferToImage(cmd, buf, image, region);

			if (mHasMips)
			{
				int32_t mipWidth = mWidth;
				int32_t mipHeight = mHeight;
				for (int i = 1; i < mMipLevels; i++)
				{
					// transition previous mip to transfer src
					context.TransitionImageLayout(
						cmd,
						this,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						i - 1,
						1,
						0,
						1);


					VkImageBlit blit{};
					blit.srcOffsets[0] = { 0, 0, 0 };
					blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
					blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.srcSubresource.mipLevel = i - 1;
					blit.srcSubresource.baseArrayLayer = 0;
					blit.srcSubresource.layerCount = 1;

					blit.dstOffsets[0] = { 0, 0, 0 };
					blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
					blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.dstSubresource.mipLevel = i;
					blit.dstSubresource.baseArrayLayer = 0;
					blit.dstSubresource.layerCount = 1;

					context.BlitMip(cmd, this, blit);


					if (mipWidth > 1) mipWidth /= 2;
					if (mipHeight > 1) mipHeight /= 2;
				}

				// transition last mip level to transfer src
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					mMipLevels - 1,
					1,
					0,
					1);

				// Since we have mips we assume its going to be used as a sampler image, transition the enitre image to shader read only
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					0,
					mMipLevels,
					0,
					1);

				SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			else
			{
				// we dont have mips but we do have data, transfer to shader read only
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					mMipLevels - 1,
					1,
					0,
					1);

				SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
		else
		{
			bool isRenderTarget = (flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget;
			bool isDepth = (flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment;
			if (isRenderTarget)
			{
				if (isDepth)
				{
					context.TransitionImageLayout(
						cmd,
						this,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						0, mMipLevels,
						0, 1);
				}
				else
				{
					context.TransitionImageLayout(
						cmd,
						this,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						0, mMipLevels,
						0, 1);
				}
			}
			else
			{
				// Its not a render target so we assume some kind of storage image
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_GENERAL,
					0, mMipLevels,
					0, 1);
			}
		}

		context.EndSingleTimeCommandBuffer(cmd);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{

	}

	uint32_t VulkanTexture2D::GetWidth()
	{
		return mWidth;
	}

	uint32_t VulkanTexture2D::GetHeight()
	{
		return mHeight;
	}

	uint32_t VulkanTexture2D::GetDepth()
	{
		return mDepth;
	}

	uint32_t VulkanTexture2D::GetMipLevels()
	{
		return mMipLevels;
	}

	uint32_t VulkanTexture2D::GetArrayLayers()
	{
		return mArrayLayers;
	}

	ImTextureID VulkanTexture2D::GetImGuiID(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return GetTextureView(mipLevel, arrayLayer)->GetImGuiID();
	}
	
	WeakRef<TextureView> VulkanTexture2D::GetView(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return GetTextureView(mipLevel, arrayLayer);
	}
}