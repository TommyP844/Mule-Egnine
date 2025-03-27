#include "Asset/Loader/MaterialLoader.h"

// Mule
#include "Asset/Loader/YamlFormatter.h"

// Submodules
#include "yaml-cpp/yaml.h"

// STD
#include <fstream>

namespace Mule
{
	Ref<Material> MaterialLoader::LoadText(const fs::path& filepath)
	{
		Ref<Material> material = MakeRef<Material>();
		material->SetFilePath(filepath);

		YAML::Node root = YAML::LoadFile(filepath.string());

		material->AlbedoColor = root["AlbedoColor"].as<glm::vec4>();

		material->AlbedoMap = root["AlbedoMap"].as<AssetHandle>();
		material->NormalMap = root["NormalMap"].as<AssetHandle>();
		material->RoughnessMap = root["RoughnessMap"].as<AssetHandle>();
		material->MetalnessMap = root["MetalnessMap"].as<AssetHandle>();
		material->AOMap = root["AOMap"].as<AssetHandle>();
		material->EmissiveMap = root["EmissiveMap"].as<AssetHandle>();
		if (root["OpacityMap"])
		{
			material->OpacityMap = root["OpacityMap"].as<AssetHandle>();
		}

		material->TextureScale = root["TextureScale"].as<glm::vec2>();
		material->Transparent = root["Transparent"].as<bool>();
		material->Transparency = root["Transparency"].as<float>();
		material->MetalnessFactor = root["MetalnessFactor"].as<float>();
		material->RoughnessFactor = root["RoughnessFactor"].as<float>();
		material->AOFactor = root["AOFactor"].as<float>();

		return material;
	}
	
	void MaterialLoader::SaveText(Ref<Material> asset)
	{
		YAML::Node root;

		root["AlbedoColor"] = asset->AlbedoColor;

		root["AlbedoMap"] = asset->AlbedoMap;
		root["NormalMap"] = asset->NormalMap;
		root["RoughnessMap"] = asset->RoughnessMap;
		root["MetalnessMap"] = asset->MetalnessMap;
		root["AOMap"] = asset->AOMap;
		root["EmissiveMap"] = asset->EmissiveMap;
		root["OpacityMap"] = asset->OpacityMap;

		root["TextureScale"] = asset->TextureScale;
		root["Transparent"] = asset->Transparent;
		root["Transparency"] = asset->Transparency;
		root["MetalnessFactor"] = asset->MetalnessFactor;
		root["RoughnessFactor"] = asset->RoughnessFactor;
		root["AOFactor"] = asset->AOFactor;

		YAML::Emitter emitter;
		emitter << root;
		std::ofstream file(asset->FilePath());
		if (!file)
		{
			SPDLOG_ERROR("Failed to open material save file: {}", asset->FilePath().string());
			return;
		}
		file << emitter.c_str();
		file.close();
	}
	
	Ref<Material> MaterialLoader::LoadBinary(const Buffer& buffer)
	{
		return Ref<Material>();
	}
	
	void MaterialLoader::SaveBinary(Ref<Material> asset)
	{
	}
}