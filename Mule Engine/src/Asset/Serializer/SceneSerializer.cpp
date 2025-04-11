#include "Asset/Serializer/SceneSerializer.h"

#include "Scripting/ScriptContext.h"
#include "ECS/Entity.h"
#include "Asset/Serializer/Convert/YamlConvert.h"
#include "Engine Context/EngineContext.h"

// Submodules
#include <spdlog/spdlog.h>

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/emitter.h>

// STD
#include <fstream>

#define SERIALIZE_COMPONENT_IF_EXISTS(name, x) if(e.HasComponent<x>()) node[name] = e.GetComponent<x>();
#define DESERIALIZE_COMPONENT_IF_EXISTS(name, x) \
    if(node[name]) { \
        auto& component = e.AddComponent<x>(); \
        component = node[name].as<x>(); \
    } \

namespace Mule
{

	SceneSerializer::SceneSerializer(WeakRef<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	Ref<Scene> SceneSerializer::Load(const fs::path& filepath)
	{
		YAML::Node root = YAML::LoadFile(filepath.string());
		
		Ref<Scene> scene = MakeRef<Scene>();
		scene->SetFilePath(filepath);

		for (auto node : root["entities"])
		{
			DeSerializeEntityYAML(node, scene);
		}

		scene->ClearModified();

		return scene;
	}

	void SceneSerializer::Save(Ref<Scene> asset)
	{
		fs::path filepath = asset->FilePath();

		YAML::Node root;

		YAML::Node entities;

		for (auto entity : asset->Iterate<RootComponent>())
		{
			entities.push_back(SerializeEntityYAML(entity));
		}

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

	YAML::Node SceneSerializer::SerializeEntityYAML(Entity e)
	{
		YAML::Node node;

		node["Name"] = e.Name();
		node["Guid"] = (size_t)e.Guid();

		node["Transform"] = e.GetComponent<Mule::TransformComponent>();

		SERIALIZE_COMPONENT_IF_EXISTS("Camera", CameraComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("EnvironmentMap", EnvironmentMapComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("Mesh", MeshComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("PointLight", PointLightComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("DirectionalLight", DirectionalLightComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("Script", ScriptComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("RigidBody", RigidBodyComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("SphereCollider", SphereColliderComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("BoxCollider", BoxColliderComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("CapsuleCollider", CapsuleColliderComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("PlaneCollider", PlaneColliderComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("RigidBodyConstraint", RigidBodyConstraintComponent);

		YAML::Node childNode;
		for (auto child : e.Children())
		{
			childNode.push_back(SerializeEntityYAML(child));
		}

		node["Children"] = childNode;

		return node;
	}

	Entity SceneSerializer::DeSerializeEntityYAML(const YAML::Node& node, WeakRef<Scene> scene)
	{
		Entity e = scene->CreateEntity(node["Name"].as<std::string>(), Mule::Guid(node["Guid"].as<uint64_t>()));
		auto& transformComponent = e.GetComponent<Mule::TransformComponent>();
		transformComponent = node["Transform"].as<Mule::TransformComponent>();

		DESERIALIZE_COMPONENT_IF_EXISTS("Camera", CameraComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("EnvironmentMap", EnvironmentMapComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("Mesh", MeshComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("DirectionalLight", DirectionalLightComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("PointLight", PointLightComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("RigidBody", RigidBodyComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("SphereCollider", SphereColliderComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("BoxCollider", BoxColliderComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("CapsuleCollider", CapsuleColliderComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("PlaneCollider", PlaneColliderComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("RigidBodyConstraint", RigidBodyConstraintComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("Script", ScriptComponent);


		for (auto childNode : node["Children"])
		{
			auto child = DeSerializeEntityYAML(childNode, scene);
			e.AddChild(child);
		}

		return e;
	}
}