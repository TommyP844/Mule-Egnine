#pragma once

#include "Mule.h"

#include <string>
#include <set>
#include <filesystem>
namespace fs = std::filesystem;

static bool IsModelExtension(const fs::path& path)
{
	std::set<std::string> extensions = { ".gltf" };
	std::string extension = path.extension().string().c_str();
	return extensions.contains(extension);
}

static bool IsTextureExtension(const fs::path& path)
{
	std::set<std::string> extensions = { ".jpg", ".jpeg", ".png", ".tga", ".bmp" };
	return extensions.contains(path.extension().string());
}

static void AddModelToEntity(Mule::Entity e, Ref<Mule::Model> model)
{

}
