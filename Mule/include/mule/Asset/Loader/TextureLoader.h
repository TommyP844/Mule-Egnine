#pragma once

#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Context/GraphicsContext.h"
#include "AssetLoader.h"

namespace Mule
{
	class TextureLoader : public IAssetLoader<Texture2D, AssetType::Texture>
	{
	public:
		virtual Ref<Texture2D> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<Texture2D> asset) override;

		virtual Ref<Texture2D> LoadBinary(const Buffer& buffer) override;
		virtual void SaveBinary(Ref<Texture2D> asset) override;

		void SetContext(Ref<GraphicsContext> context);
	private:
		Ref<GraphicsContext> mContext;
	};
}