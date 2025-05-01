#pragma once

#include "WeakRef.h"
#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/TextureView.h"

#include "Asset/Asset.h"

#include "Buffer.h"

#include "imgui.h"

namespace Mule
{
	class Texture : public Asset<AssetType::Texture>
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
		virtual uint32_t GetDepth() = 0;
		virtual uint32_t GetMipLevels() = 0;
		virtual uint32_t GetArrayLayers() = 0;
		virtual ImTextureID GetImGuiID(uint32_t mipLevel = 0, uint32_t arrayLayer = 0) const = 0;
		virtual WeakRef<TextureView> GetView(uint32_t mipLevel, uint32_t arrayLayer = 0) const = 0;
		virtual WeakRef<TextureView> GetMipView(uint32_t mipLevel) = 0;
		virtual void TransitionImageLayoutImmediate(ImageLayout newLayout) = 0;
		virtual Buffer ReadTextureData(uint32_t mipLevel = 0) = 0;
		virtual void WriteMipLevel(uint32_t mipLevel, const Buffer& data) = 0;
		
		TextureFormat GetFormat() const { return mFormat; }
		TextureFlags GetFlags() const { return mFlags; }
		TextureType GetType() const { return mType; }

	protected:
		Texture(const std::string& name, TextureFormat format, TextureFlags flags, TextureType type)
			:
			mFormat(format),
			mFlags(flags),
			mType(type),
			Asset(name)
		{ }

	private:
		TextureFormat mFormat;
		TextureFlags mFlags;
		TextureType mType;
	};
}
