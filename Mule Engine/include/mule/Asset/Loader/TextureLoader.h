#pragma once

#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Context/GraphicsContext.h"
#include "AssetLoader.h"

namespace Mule
{
	class TextureLoader : public IAssetLoader<Texture2D, AssetType::Texture>
	{
	public:
		TextureLoader(WeakRef<GraphicsContext> context);

		virtual Ref<Texture2D> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<Texture2D> asset) override;

		virtual Ref<Texture2D> LoadBinary(const Buffer& buffer) override;
		virtual void SaveBinary(Ref<Texture2D> asset) override;

		
	private:
		WeakRef<GraphicsContext> mContext;
	};
}