#pragma once

#include "Asset/Asset.h"

#include "Ref.h"
#include "Graphics/Texture/TextureCube.h"

namespace Mule
{
	class EnvironmentMap : public Asset<AssetType::EnvironmentMap>
	{
	public:
		EnvironmentMap(WeakRef<GraphicsContext> context, const fs::path& environmentMapHdr);

	private:
		Ref<TextureCube> mCubemap;
	};
}
