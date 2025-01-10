#pragma once

#include "Asset/Asset.h"

namespace Mule
{
	class Model : public Asset<AssetType::Model>
	{
	public:
		Model() : Asset() {}
	};
}