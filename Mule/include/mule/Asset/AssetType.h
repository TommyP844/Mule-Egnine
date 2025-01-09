#pragma once

#include <string>

namespace Mule
{
	enum AssetType
	{
		Texture,
		Scene,
		Model,
		Mesh,
		Material
	};

	static std::string GetAssetTypeString(AssetType type)
	{
		switch (type)
		{
		case Mule::Texture: return "Texture";
		case Mule::Scene: return "Scene";
		case Mule::Model: return "Model";
		case Mule::Mesh: return "Mesh";
		case Mule::Material: return "Material";
		}

		return "Unknown";
	}
}