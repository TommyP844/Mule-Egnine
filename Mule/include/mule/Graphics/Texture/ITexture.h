#pragma once

#include "WeakRef.h"
#include "Graphics/RenderTypes.h"

#include "Asset/Asset.h"

#include <imgui.h>

namespace Mule
{
	enum class TextureType {
		Type_1D,
		Type_1DArray,
		Type_2D,
		Type_2DArray,
		Type_3D,
		Type_Cube,
		Type_CubeArray,
	};

	class GraphicsContext;

	class ITexture : public Asset<AssetType::Texture>
	{
	public:
		ITexture(WeakRef<GraphicsContext> context);
		ITexture(WeakRef<GraphicsContext> context, const fs::path& filepath, AssetHandle handle = GenerateUUID());
		ITexture(WeakRef<GraphicsContext> context, const std::string& name);
		virtual ~ITexture();

		void SetImageLayout(VkImageLayout layout) { mVulkanImage.Layout = layout; }

		VkImage GetImage() const { return mVulkanImage.Image; }
		VkImageView GetImageView() const { return mVulkanImage.ImageView; }

		TextureFormat GetFormat() const { return mFormat; }
		uint32_t GetLayerCount() const { return mLayers; }
		uint32_t GetMipCount() const { return mMips; }
		uint32_t GetWidth() const { return mWidth; }
		uint32_t GetHeight() const { return mHeight; }
		uint32_t GetDepth() const { return mDepth; }
		bool IsDepthTexture() const { return mIsDepthTexture; }

		const VulkanImage& GetVulkanImage() const { return mVulkanImage; }

		TextureType GetTextureType() const { return mTextureType; }

		ImTextureID GetLayerID(int index);

	protected:
		void Initialize(void* data, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t mips, TextureFormat format, TextureFlags flags);

		WeakRef<GraphicsContext> mContext;
		VkDevice mDevice;
		VulkanImage mVulkanImage;

		struct ImGuiLayerView
		{
			VkImageView ImageView;
			ImTextureID Id;
		};

		std::vector<ImGuiLayerView> mLayerViews;
		
		bool mIsDepthTexture;
		uint32_t mWidth, mHeight, mDepth, mMips, mLayers;
		TextureFormat mFormat;
		TextureType mTextureType;
	};
}