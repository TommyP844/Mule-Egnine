#include "Asset/Loader/SceneLoader.h"

#include "ECS/Entity.h"
#include "Asset/Loader/YamlFormatter.h"

// Submodules
#include <spdlog/spdlog.h>

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/emitter.h>

// STD
#include <fstream>

namespace Mule
{
	static Entity DeserializeEntity(YAML::Node node)
	{
		auto parent = node.as<Mule::Entity>();
		for (auto child : node["Children"])
		{
			auto e = DeserializeEntity(child);
			parent.AddChild(e);
		}
		return parent;
	}

	Ref<Scene> SceneLoader::LoadText(const fs::path& filepath)
	{
		YAML::Node root = YAML::LoadFile(filepath.string());
		
		Ref<Scene> scene = MakeRef<Scene>();
		scene->SetFilePath(filepath);

		YAML::convert<Mule::Entity>::gScene = scene;

		for (auto node : root["entities"])
		{
			DeserializeEntity(node);
		}

		scene->ClearModified();

		return scene;
	}

	void SceneLoader::SaveText(Ref<Scene> asset)
	{
		fs::path filepath = asset->FilePath();

		YAML::Node root;

		YAML::Node entities;

		asset->IterateRootEntities([&](Entity e) {
			entities.push_back(e);
			});

		root["entities"] = entities;

		YAML::Emitter emitter;
		emitter << root;
	
		std::ofstream file(asset->FilePath());
		if (!file.is_open())
		{
			SPDLOG_ERROR("Failed to open save file: {}", filepath.string());
		}
		file << emitter.c_str();
		file.close();
	}

	Ref<Scene> SceneLoader::LoadBinary(const Buffer& filepath)
	{
		return Ref<Scene>();
	}

	void SceneLoader::SaveBinary(Ref<Scene> asset)
	{
	}
}