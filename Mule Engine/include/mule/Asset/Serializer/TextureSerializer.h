#pragma once

#include "Graphics/API/Texture2D.h"
#include "IAssetSerializer.h"

namespace Mule
{
	class TextureSerializer : public IAssetSerializer<Texture2D, AssetType::Texture>
	{
	public:
		TextureSerializer(Ref<ServiceManager> serviceManager);

		virtual Ref<Texture2D> Load(const fs::path& filepath) override;
		virtual void Save(Ref<Texture2D> asset) override;

	};
}