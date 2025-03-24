#include "Asset/Loader/SceneLoader.h"

#include "Scripting/ScriptContext.h"
#include "ECS/Entity.h"
#include "Asset/Loader/YamlFormatter.h"
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

	SceneLoader::SceneLoader(WeakRef<EngineContext> engineContext, WeakRef<ScriptContext> scriptContext)
		:
		mScriptContext(scriptContext),
		mEngineContext(engineContext)
	{
		YAML::convert<Mule::ScriptComponent>::gScriptContext = mScriptContext;
	}

	Ref<Scene> SceneLoader::LoadText(const fs::path& filepath)
	{
		YAML::Node root = YAML::LoadFile(filepath.string());
		
		Ref<Scene> scene = MakeRef<Scene>(mEngineContext);
		scene->SetFilePath(filepath);
		mEngineContext->SetScene(scene);


		for (auto node : root["entities"])
		{
			DeSerializeEntityYAML(node, scene);
		}

		scene->ClearModified();
		mEngineContext->SetScene(nullptr);

		return scene;
	}

	void SceneLoader::SaveText(Ref<Scene> asset)
	{
		fs::path filepath = asset->FilePath();

		YAML::Node root;

		YAML::Node entities;

		asset->IterateRootEntities([&](Entity e) {
			entities.push_back(SerializeEntityYAML(e));
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

	YAML::Node SceneLoader::SerializeEntityYAML(Entity e)
	{
		YAML::Node node;

		node["Name"] = e.Name();
		node["Guid"] = (size_t)e.Guid();

		node["Transform"] = e.GetComponent<Mule::TransformComponent>();

		SERIALIZE_COMPONENT_IF_EXISTS("Camera", Mule::CameraComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("EnvironmentMap", Mule::EnvironmentMapComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("Mesh", Mule::MeshComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("PointLight", Mule::PointLightComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("DirectionalLight", Mule::DirectionalLightComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("Script", Mule::ScriptComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("RigidBody3D", Mule::RigidBody3DComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("SphereCollider", Mule::SphereColliderComponent);
		SERIALIZE_COMPONENT_IF_EXISTS("BoxCollider", Mule::BoxColliderComponent);

		YAML::Node childNode;
		for (auto child : e.Children())
		{
			childNode.push_back(SerializeEntityYAML(child));
		}

		node["Children"] = childNode;

		return node;
	}

	Entity SceneLoader::DeSerializeEntityYAML(const YAML::Node& node, WeakRef<Scene> scene)
	{
		Entity e = scene->CreateEntity(node["Name"].as<std::string>(), Mule::Guid(node["Guid"].as<uint64_t>()));
		auto& transformComponent = e.GetComponent<Mule::TransformComponent>();
		transformComponent = node["Transform"].as<Mule::TransformComponent>();

		DESERIALIZE_COMPONENT_IF_EXISTS("Camera", Mule::CameraComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("EnvironmentMap", Mule::EnvironmentMapComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("Mesh", Mule::MeshComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("DirectionalLight", Mule::DirectionalLightComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("PointLight", Mule::PointLightComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("RigidBody3D", Mule::RigidBody3DComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("SphereCollider", Mule::SphereColliderComponent);
		DESERIALIZE_COMPONENT_IF_EXISTS("BoxCollider", Mule::BoxColliderComponent);

		if (node["Script"])
		{
			DeserializeScriptComponentYAML(node["Script"], e);
		}

		for (auto childNode : node["Children"])
		{
			auto child = DeSerializeEntityYAML(childNode, scene);
			e.AddChild(child);
		}

		return e;
	}

	void SceneLoader::DeserializeScriptComponentYAML(const YAML::Node& node, Entity e)
	{
		if (!node["Class"]) return;

		std::string className = node["Class"].as<std::string>();
		Mule::ScriptHandle handle = node["Handle"].as<Mule::ScriptHandle>();

		bool success = mScriptContext->CreateInstance(className, e.ID(), handle);
		if (success)
		{
			auto& scriptComponent = e.AddComponent<ScriptComponent>();
			scriptComponent.Handle = handle;

			const auto& scriptType = mScriptContext->GetType(className);
			auto scriptInstance = mScriptContext->GetScriptInstance(handle);

			for (auto fieldNode : node["Fields"])
			{
				std::string fieldName = fieldNode["Name"].as<std::string>();
				Mule::ScriptFieldType fieldType = (Mule::ScriptFieldType)fieldNode["Type"].as<uint32_t>();

				if (!scriptType.DoesFieldExist(fieldName))
					continue;

				const auto& field = scriptType.GetField(fieldName);
				if (field.Type != fieldType)
					continue;


#define DESERIALIZE_SCRIPT_FIELD(type) scriptInstance->SetFieldValue<type>(fieldName, fieldNode["Value"].as<type>());
				switch (fieldType)
				{
				case Mule::ScriptFieldType::Bool: DESERIALIZE_SCRIPT_FIELD(bool); break;
				case Mule::ScriptFieldType::Int16: DESERIALIZE_SCRIPT_FIELD(int16_t); break;
				case Mule::ScriptFieldType::Int32: DESERIALIZE_SCRIPT_FIELD(int32_t); break;
				case Mule::ScriptFieldType::Int64: DESERIALIZE_SCRIPT_FIELD(int64_t); break;
				case Mule::ScriptFieldType::UInt16: DESERIALIZE_SCRIPT_FIELD(uint16_t); break;
				case Mule::ScriptFieldType::UInt32: DESERIALIZE_SCRIPT_FIELD(uint32_t); break;
				case Mule::ScriptFieldType::UInt64: DESERIALIZE_SCRIPT_FIELD(uint64_t); break;
				case Mule::ScriptFieldType::Float: DESERIALIZE_SCRIPT_FIELD(float); break;
				case Mule::ScriptFieldType::Double: DESERIALIZE_SCRIPT_FIELD(double); break;
					//case Mule::FieldType::Decimal: SERIALIZE_SCRIPT_FIELD_VALUE(bool); break;
					//case Mule::FieldType::UIntPtr: SERIALIZE_SCRIPT_FIELD_VALUE(unsigned long*); break;
					//case Mule::FieldType::IntPtr: SERIALIZE_SCRIPT_FIELD_VALUE(bool); break;
				}
			}
		}
	}
}