
#include "Graphics/API/Vulkan/IVulkanTexture.h"
#include "Graphics/API/Vulkan/VulkanGraphicsContext.h"

#include <spdlog/spdlog.h>
#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

#include <set>


namespace Mule

{
	IVulkanTexture::IVulkanTexture(WeakRef<GraphicsContext> context)
		:
		Asset(),
		mContext(context),
		mDevice(context->GetDevice()),
		mIsDepthTexture(false)
	{
	}

	IVulkanTexture::IVulkanTexture(WeakRef<GraphicsContext> context, const fs::path& filepath, AssetHandle handle)
		:
		Asset(handle, filepath),
		mContext(context),
		mDevice(context->GetDevice()),
		mIsDepthTexture(false)
	{
	}

	IVulkanTexture::IVulkanTexture(WeakRef<GraphicsContext> context, const std::string& name)
		:
		Asset(name),
		mContext(context),
		mDevice(context->GetDevice()),
		mIsDepthTexture(false)
	{
	}

	IVulkanTexture::~IVulkanTexture()
	{
		vkDeviceWaitIdle(mDevice);

		vkDestroySampler(mDevice, mSampler, nullptr);

		for (auto mipView : mMipViews)
		{
			vkDestroyImageView(mDevice, mipView.View, nullptr);
			for (auto layer : mipView.LayerViews)
			{
				vkDestroyImageView(mDevice, layer, nullptr);
			}
		}

		vkFreeMemory(mDevice, mVulkanImage.Memory, nullptr);
		vkDestroyImageView(mDevice, mVulkanImage.ImageView, nullptr);
		vkDestroyImage(mDevice, mVulkanImage.Image, nullptr);
	}

	VkImageView IVulkanTexture::GetMipLayerImageView(uint32_t mipLevel, uint32_t layer) const
	{
		return mMipViews[mipLevel].LayerViews[layer];
	}

	VkImageView IVulkanTexture::GetMipImageView(uint32_t mipLevel) const
	{
		return mMipViews[mipLevel].View;
	}

	ImTextureID IVulkanTexture::GetImGuiMipLayerID(uint32_t mipLevel, uint32_t layer) const
	{
		return mMipViews[mipLevel].ImGuiLayerIDs[layer];
	}

	ImTextureID IVulkanTexture::GetImGuiMipID(uint32_t mipLevel) const
	{
		return mMipViews[mipLevel].ImGuiMipId;
	}

	glm::ivec2 IVulkanTexture::GetMipLevelSize(uint32_t mipLevel) const
	{
		return glm::ivec2(mWidth / glm::pow(2, mipLevel), mHeight / glm::pow(2, mipLevel));
	}

	void IVulkanTexture::GenerateMips()
	{
		auto queue = mContext->GetGraphicsQueue();
		auto commandPool = queue->CreateCommandPool();
		auto commandBuffer = commandPool->CreateCommandBuffer();
		commandBuffer->Begin();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = mVulkanImage.Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mMips;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = mLayers;
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(
			commandBuffer->GetHandle(),
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		int32_t mipWidth = mWidth;
		int32_t mipHeight = mHeight;
		for (int i = 1; i < mMips; i++)
		{
			// Transition previous mip level to TRANSFER_SRC_OPTIMAL
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.subresourceRange.levelCount = 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer->GetHandle(),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = mLayers;

			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = mLayers;

			vkCmdBlitImage(
				commandBuffer->GetHandle(),
				mVulkanImage.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				mVulkanImage.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR
			);

			// Transition previous mip level to SHADER_READ_ONLY_OPTIMAL
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer->GetHandle(),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		// Final transition for the last mip level to SHADER_READ_ONLY_OPTIMAL
		barrier.subresourceRange.baseMipLevel = mMips - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer->GetHandle(),
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		commandBuffer->End();
		auto fence = mContext->CreateFence();
		fence->Reset();
		queue->Submit(commandBuffer, {}, {}, fence);
		fence->Wait();

	}

	void IVulkanTexture::Initialize(void* data, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, TextureFormat format, TextureFlags flags)
	{
		// TODO: deduce image type
		VkImageType imageType;
		VkImageViewType viewType;

		if (width > 1 && height == 1 && depth == 1)
		{
			mTextureType = TextureType::Type_1D;
			imageType = VK_IMAGE_TYPE_1D;
			viewType = VK_IMAGE_VIEW_TYPE_1D;
			if (layers > 1)
			{
				mTextureType = TextureType::Type_1DArray;
				viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			}
		}
		else if (width > 1 && height > 1 && depth == 1)
		{
			mTextureType = TextureType::Type_2D;
			imageType = VK_IMAGE_TYPE_2D;
			viewType = VK_IMAGE_VIEW_TYPE_2D;
			if (layers > 1)
			{
				mTextureType = TextureType::Type_2DArray;
				viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			}
			
			if (flags & TextureFlags::CubeMap)
			{
				layers *= 6;
				viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				mTextureType = TextureType::Type_Cube;
			}
		}
		else if (width > 1 && height > 1 && depth > 1)
		{
			imageType = VK_IMAGE_TYPE_3D;
			viewType = VK_IMAGE_VIEW_TYPE_3D;
			mTextureType = TextureType::Type_3D;
			if (layers > 1)
			{
				SPDLOG_WARN("3D Textures connot have layers, defaulting to 1");
				layers = 1;
			}
		}

		mWidth = width;
		mHeight = height;
		mDepth = depth;
		mLayers = layers;
		mFormat = format;

		if (TextureFlags::GenerateMips & flags)
		{
			mMips = glm::log2(glm::max<float>(mWidth, mHeight));
		}
		else
		{
			mMips = 1;
		}

		mIsDepthTexture = ((flags & TextureFlags::DepthTexture) == TextureFlags::DepthTexture);
		VkImageAspectFlags imageAspect = mIsDepthTexture ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;;


		std::set<uint32_t> queueFamilyIndicexSet = {
			mContext->GetGraphicsQueue()->GetQueueFamilyIndex()
		};

		std::vector<uint32_t> queueFamilyIndices;
		std::copy(queueFamilyIndicexSet.begin(), queueFamilyIndicexSet.end(), std::back_inserter(queueFamilyIndices));

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if ((flags & TextureFlags::RenderTarget) && (flags & TextureFlags::DepthTexture))
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else if(flags & TextureFlags::RenderTarget)
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (flags & TextureFlags::StorageImage)
			usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		
		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = (flags & TextureFlags::CubeMap) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
		info.imageType = imageType;
		info.format = (VkFormat)format;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = depth;
		info.mipLevels = mMips;
		info.arrayLayers = layers;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usageFlags;
		info.sharingMode = queueFamilyIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = queueFamilyIndices.size();
		info.pQueueFamilyIndices = queueFamilyIndices.data();
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(mDevice, &info, nullptr, &mVulkanImage.Image);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to create image");
			return;
		}

		VkMemoryRequirements requierments;
		vkGetImageMemoryRequirements(mDevice, mVulkanImage.Image, &requierments);

		uint32_t memoryTypeIndex = mContext->GetMemoryTypeIndex(requierments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = requierments.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		allocInfo.pNext = nullptr;

		result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &mVulkanImage.Memory);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to allocate device memory");
		}

		result = vkBindImageMemory(mDevice, mVulkanImage.Image, mVulkanImage.Memory, 0);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to bind image memory");
			return;
		}

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = mVulkanImage.Image;
		viewCreateInfo.viewType = viewType;
		viewCreateInfo.format = (VkFormat)format;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = imageAspect;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = mMips;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = layers;

		result = vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &mVulkanImage.ImageView);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create image view");
		}

		// Sampler
		{
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
			samplerCreateInfo.maxAnisotropy = 16.f;
			samplerCreateInfo.compareEnable = VK_FALSE;
			// samplerCreateInfo.compareOp;
			samplerCreateInfo.minLod = 0;
			samplerCreateInfo.maxLod = mMips;
			samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

			result = vkCreateSampler(mDevice, &samplerCreateInfo, nullptr, &mSampler);
			if (result != VK_SUCCESS)
			{
				SPDLOG_ERROR("Failed to create linear sampler");
			}
		}

		for (int mip = 0; mip < mMips; mip++)
		{
			viewCreateInfo.viewType = viewType;
			viewCreateInfo.subresourceRange.baseMipLevel = mip;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = layers;

			MipView mipView;

			vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &mipView.View);

			mipView.LayerViews.resize(mLayers);
			mipView.ImGuiLayerIDs.resize(mLayers);

			for (uint32_t layer = 0; layer < mLayers; layer++)
			{
				if (flags & TextureFlags::CubeMap)
				{
					viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				}

				viewCreateInfo.subresourceRange.baseArrayLayer = layer;
				viewCreateInfo.subresourceRange.layerCount = 1;

				vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &mipView.LayerViews[layer]);

				mipView.ImGuiLayerIDs[layer] = (ImTextureID)ImGui_ImplVulkan_AddTexture(mSampler, mipView.LayerViews[layer], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			mMipViews.push_back(mipView);
		}

		auto queue = mContext->GetGraphicsQueue();
		auto commandPool = queue->CreateCommandPool();
		auto commandBuffer = commandPool->CreateCommandBuffer();
		commandBuffer->Begin();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = mVulkanImage.Image;
		barrier.subresourceRange.aspectMask = imageAspect;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mMips;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = mLayers;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(
			commandBuffer->GetHandle(),
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
		if (data != nullptr)
		{
			size_t size = width * height * depth * layers * GetFormatSize(mFormat);

			VkBufferCreateInfo bufferCreateInfo = {};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = size;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			
			vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &stagingBuffer);

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(mDevice, stagingBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = mContext->GetMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			vkAllocateMemory(mDevice, &allocInfo, nullptr, &stagingBufferMemory);
			vkBindBufferMemory(mDevice, stagingBuffer, stagingBufferMemory, 0);

			void* ptr;
			vkMapMemory(mDevice, stagingBufferMemory, 0, size, 0, &ptr);
			memcpy(ptr, data, size);
			vkUnmapMemory(mDevice, stagingBufferMemory);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = imageAspect;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = mLayers;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { mWidth, mHeight, mDepth };

			vkCmdCopyBufferToImage(
				commandBuffer->GetHandle(),
				stagingBuffer,
				mVulkanImage.Image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region);

			if (flags & TextureFlags::GenerateMips && mMips > 1)
			{
				int32_t mipWidth = mWidth;
				int32_t mipHeight = mHeight;
				for (int i = 1; i < mMips; i++)
				{
					barrier.subresourceRange.baseMipLevel = i-1;
					barrier.subresourceRange.levelCount = 1;
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

					vkCmdPipelineBarrier(
						commandBuffer->GetHandle(),
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
						0, nullptr,
						0, nullptr,
						1, &barrier);

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

					vkCmdBlitImage(
						commandBuffer->GetHandle(),
						mVulkanImage.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						mVulkanImage.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1, &blit,
						VK_FILTER_LINEAR);

					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					vkCmdPipelineBarrier(commandBuffer->GetHandle(),
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						0, nullptr,
						0, nullptr,
						1, &barrier);

					if (mipWidth > 1) mipWidth /= 2;
					if (mipHeight > 1) mipHeight /= 2;
				}

				barrier.subresourceRange.baseMipLevel = mMips - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer->GetHandle(),
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);
			}
			else
			{
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = mVulkanImage.Image;
				barrier.subresourceRange.aspectMask = imageAspect;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = mMips;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = mLayers;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(
					commandBuffer->GetHandle(),
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

			mVulkanImage.Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			commandBuffer->End();
			auto fence = mContext->CreateFence();
			fence->Reset();
			queue->Submit(commandBuffer, {}, {}, fence);
			fence->Wait();
		}
		else
		{

			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = mVulkanImage.Image;
			barrier.subresourceRange.aspectMask = imageAspect;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mMips;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = mLayers;

			VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			VkPipelineStageFlags dstStage;

			if (flags & TextureFlags::RenderTarget)
			{
				if (flags & TextureFlags::DepthTexture)
				{
					barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
					barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

					dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				}
				else
				{
					barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

					dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
			}
			else
			{
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}

			mVulkanImage.Layout = barrier.newLayout;

			vkCmdPipelineBarrier(
				commandBuffer->GetHandle(),
				srcStage,
				dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			commandBuffer->End();
			auto fence = mContext->CreateFence();
			fence->Reset();
			queue->Submit(commandBuffer, {}, {}, fence);
			fence->Wait();
		}

		if (stagingBufferMemory)
		{
			vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
			vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		}
	}
}