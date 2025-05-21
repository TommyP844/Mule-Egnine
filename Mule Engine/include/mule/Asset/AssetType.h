#pragma once

#include <string>

namespace Mule
{
	enum class AssetType : unsigned int
	{
		Texture,
		Scene,
		Model,
		Mesh,
		Material,
		Shader,
		ComputeShader,
		EnvironmentMap,

		Script,

		// UI
		UIStyle,
		UIPanel,
		UIElement,
		UIScene,
		UIFont,

		None
	};

	static std::string GetAssetTypeString(AssetType type)
	{
		switch (type)
		{
		case AssetType::Texture: return "Texture";
		case AssetType::Scene: return "Scene";
		case AssetType::Model: return "Model";
		case AssetType::Mesh: return "Mesh";
		case AssetType::Material: return "Material";
		case AssetType::Shader: return "Shader";
		case AssetType::EnvironmentMap: return "Environment Map";
		case AssetType::Script: return "Script";
		}

		return "Unknown";
	}
}