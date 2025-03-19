#pragma once

#include <yaml-cpp/yaml.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ECS/Components.h"

namespace YAML {

    // Serializer for glm::vec2
    template<>
    struct convert<glm::vec2> {
        static Node encode(const glm::vec2& vec) {
            Node node;
            node.push_back(vec.x);
            node.push_back(vec.y);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& vec) {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            vec.x = node[0].as<float>();
            vec.y = node[1].as<float>();
            return true;
        }
    };

    // Serializer for glm::vec3
    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& vec) {
            Node node;
            node.push_back(vec.x);
            node.push_back(vec.y);
            node.push_back(vec.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& vec) {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            vec.x = node[0].as<float>();
            vec.y = node[1].as<float>();
            vec.z = node[2].as<float>();
            return true;
        }
    };

    // Serializer for glm::vec4
    template<>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4& vec) {
            Node node;
            node.push_back(vec.x);
            node.push_back(vec.y);
            node.push_back(vec.z);
            node.push_back(vec.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& vec) {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            vec.x = node[0].as<float>();
            vec.y = node[1].as<float>();
            vec.z = node[2].as<float>();
            vec.w = node[3].as<float>();
            return true;
        }
    };

    // Serializer for glm::quat
    template<>
    struct convert<glm::quat> {
        static Node encode(const glm::quat& quat) {
            Node node;
            node.push_back(quat.x);
            node.push_back(quat.y);
            node.push_back(quat.z);
            node.push_back(quat.w);
            return node;
        }

        static bool decode(const Node& node, glm::quat& quat) {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            quat.x = node[0].as<float>();
            quat.y = node[1].as<float>();
            quat.z = node[2].as<float>();
            quat.w = node[3].as<float>();
            return true;
        }
    };

    // Serializer for glm::mat3
    template<>
    struct convert<glm::mat3> {
        static Node encode(const glm::mat3& mat) {
            Node node;
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    node[i][j] = mat[i][j];
                }
            }
            return node;
        }

        static bool decode(const Node& node, glm::mat3& mat) {
            if (!node.IsMap())
                return false;

            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    mat[i][j] = node[i][j].as<float>();
                }
            }
            return true;
        }
    };

    // Serializer for glm::mat4
    template<>
    struct convert<glm::mat4> {
        static Node encode(const glm::mat4& mat) {
            Node node;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    node[i][j] = mat[i][j];
                }
            }
            return node;
        }

        static bool decode(const Node& node, glm::mat4& mat) {
            if (!node.IsMap())
                return false;

            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    mat[i][j] = node[i][j].as<float>();
                }
            }
            return true;
        }
    };

    template<>
    struct convert<fs::path> {
        static Node encode(const fs::path& path) {
            Node node;
            node = path.string();
            return node;
        }

        static bool decode(const Node& node, fs::path& path) {

            path = node.as<std::string>();
            return true;
        }
    };

#pragma region Components

    template<>
    struct convert<Mule::TransformComponent> {
        static Node encode(const Mule::TransformComponent& transform) {
            Node node;

            node["Translation"] = transform.Translation;
            node["Rotation"] = transform.Rotation;
            node["Scale"] = transform.Scale;

            return node;
        }

        static bool decode(const Node& node, Mule::TransformComponent& transform) {

            transform.Translation = node["Translation"].as<glm::vec3>();
            transform.Rotation = node["Rotation"].as<glm::vec3>();
            transform.Scale = node["Scale"].as<glm::vec3>();

            return true;
        }
    };

    template<>
    struct convert<Mule::CameraComponent> {
        static Node encode(const Mule::CameraComponent& camera) {
            Node node;

            node["Active"] = camera.Active;
            node["NearPlane"] = camera.Camera.GetNearPlane();
            node["FarPlane"] = camera.Camera.GetFarPlane();
            node["FOV"] = camera.Camera.GetFOVDegrees();
            node["Yaw"] = camera.Camera.GetYaw();
            node["Pitch"] = camera.Camera.GetPitch();
            node["WorldUp"] = camera.Camera.GetWorldUp();
            node["Position"] = camera.Camera.GetPosition();

            return node;
        }

        static bool decode(const Node& node, Mule::CameraComponent& camera) {

            camera.Active = node["Active"].as<bool>();
            
            camera.Camera.SetNearPlane(node["NearPlane"].as<float>());
            camera.Camera.SetFarPlane(node["FarPlane"].as<float>());
            camera.Camera.SetFOVDegrees(node["FOV"].as<float>());
            camera.Camera.SetYaw(node["Yaw"].as<float>());
            camera.Camera.SetPitch(node["Pitch"].as<float>());
            camera.Camera.SetWorldUp(node["WorldUp"].as<glm::vec3>());
            camera.Camera.SetPosition(node["Position"].as<glm::vec3>());

            return true;
        }
    };

    template<>
    struct convert<Mule::EnvironmentMapComponent> {
        static Node encode(const Mule::EnvironmentMapComponent& envMap) {
            Node node;

            node["Active"] = envMap.Active;
            node["Radiance"] = envMap.Radiance;
            node["EnvironmentMapHandle"] = envMap.EnvironmentMap;

            return node;
        }

        static bool decode(const Node& node, Mule::EnvironmentMapComponent& envMap) {

            envMap.Active = node["Active"].as<bool>();
            envMap.Radiance = node["Radiance"].as<float>();
            envMap.EnvironmentMap = node["EnvironmentMapHandle"].as<Mule::AssetHandle>();

            return true;
        }
    };

    template<>
    struct convert<Mule::MeshComponent> {
        static Node encode(const Mule::MeshComponent& mesh) {
            Node node;

            node["Visible"] = mesh.Visible;
            node["MeshHandle"] = mesh.MeshHandle;
            node["MaterialHandle"] = mesh.MaterialHandle;

            return node;
        }

        static bool decode(const Node& node, Mule::MeshComponent& mesh) {

            mesh.Visible = node["Visible"].as<bool>();
            mesh.MeshHandle = node["MeshHandle"].as<Mule::AssetHandle>();
            mesh.MaterialHandle = node["MaterialHandle"].as<Mule::AssetHandle>();

            return true;
        }
    };

    template<>
    struct convert<Mule::DirectionalLightComponent> {
        static Node encode(const Mule::DirectionalLightComponent& light) {
            Node node;

            node["Active"] = light.Active;
            node["Color"] = light.Color;
            node["Intensity"] = light.Intensity;

            return node;
        }

        static bool decode(const Node& node, Mule::DirectionalLightComponent& light) {

            light.Active = node["Active"].as<bool>();
            light.Color = node["Color"].as<glm::vec3>();
            light.Intensity = node["Intensity"].as<float>();

            return true;
        }
    };

    template<>
    struct convert<Mule::PointLightComponent> {
        static Node encode(const Mule::PointLightComponent& light) {
            Node node;

            node["Active"] = light.Active;
            node["Color"] = light.Color;
            node["Radiance"] = light.Radiance;

            return node;
        }

        static bool decode(const Node& node, Mule::PointLightComponent& light) {

            light.Active = node["Active"].as<bool>();
            light.Color = node["Color"].as<glm::vec3>();
            light.Radiance = node["Radiance"].as<float>();

            return true;
        }
    };

    template<>
    struct convert<Mule::ScriptComponent> {
        static Node encode(const Mule::ScriptComponent& script) {
            Node node;

            auto scriptInstance = gScriptContext->GetScriptInstance(script.Handle);

            if (scriptInstance)
            {
                node["Class"] = scriptInstance->GetName();
                node["Handle"] = script.Handle;

                node["Fields"];

                for (const auto& [name, field] : gScriptContext->GetType(scriptInstance->GetName()).GetFields())
                {
                    Node fieldNode;

                    fieldNode["Type"] = (uint32_t)field.Type;
                    fieldNode["Name"] = field.Name;

#define SERIALIZE_SCRIPT_FIELD_VALUE(type) fieldNode["Value"] = scriptInstance->GetFieldValue<type>(field.Name);
                    switch (field.Type)
                    {
                    case Mule::ScriptFieldType::Bool: SERIALIZE_SCRIPT_FIELD_VALUE(bool); break;
                    case Mule::ScriptFieldType::Int16: SERIALIZE_SCRIPT_FIELD_VALUE(int16_t); break;
                    case Mule::ScriptFieldType::Int32: SERIALIZE_SCRIPT_FIELD_VALUE(int32_t); break;
                    case Mule::ScriptFieldType::Int64: SERIALIZE_SCRIPT_FIELD_VALUE(int64_t); break;
                    case Mule::ScriptFieldType::UInt16: SERIALIZE_SCRIPT_FIELD_VALUE(uint16_t); break;
                    case Mule::ScriptFieldType::UInt32: SERIALIZE_SCRIPT_FIELD_VALUE(uint32_t); break;
                    case Mule::ScriptFieldType::UInt64: SERIALIZE_SCRIPT_FIELD_VALUE(uint64_t); break;
                    case Mule::ScriptFieldType::Float: SERIALIZE_SCRIPT_FIELD_VALUE(float); break;
                    case Mule::ScriptFieldType::Double: SERIALIZE_SCRIPT_FIELD_VALUE(double); break;
                        //case Mule::FieldType::Decimal: SERIALIZE_SCRIPT_FIELD_VALUE(bool); break;
                        //case Mule::FieldType::UIntPtr: SERIALIZE_SCRIPT_FIELD_VALUE(unsigned long*); break;
                        //case Mule::FieldType::IntPtr: SERIALIZE_SCRIPT_FIELD_VALUE(bool); break;
                    }


                    node["Fields"].push_back(fieldNode);
                }
            }

            return node;
        }

        static bool decode(const Node& node, Mule::ScriptComponent& script) {

            if (!node["Class"]) return true;

            std::string className = node["Class"].as<std::string>();
            Mule::ScriptHandle handle = node["Handle"].as<Mule::ScriptHandle>();

            bool success = gScriptContext->CreateInstance(className, gEntity.ID(), handle);
            if (success)
            {
                const auto& scriptType = gScriptContext->GetType(className);
                script.Handle = handle;
                auto scriptInstance = gScriptContext->GetScriptInstance(handle);

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

            return true;
        }

        static WeakRef<Mule::ScriptContext> gScriptContext;
        static Mule::Entity gEntity;
    };

#pragma endregion


#define SERIALIZE_COMPONENT_IF_EXISTS(name, x) if(e.HasComponent<x>()) node[name] = e.GetComponent<x>();
#define DESERIALIZE_COMPONENT_IF_EXISTS(name, x) \
    if(node[name]) { \
        auto& component = e.AddComponent<x>(); \
        component = node[name].as<x>(); \
    } \

    template<>
    struct convert<Mule::Entity> {
        static Node encode(const Mule::Entity& e) {
            Node node;

            node["Name"] = e.Name();
            node["Guid"] = (size_t)e.Guid();

            node["Transform"] = e.GetComponent<Mule::TransformComponent>();

            SERIALIZE_COMPONENT_IF_EXISTS("Camera", Mule::CameraComponent);
            SERIALIZE_COMPONENT_IF_EXISTS("EnvironmentMap", Mule::EnvironmentMapComponent);
            SERIALIZE_COMPONENT_IF_EXISTS("Mesh", Mule::MeshComponent);
            SERIALIZE_COMPONENT_IF_EXISTS("PointLight", Mule::PointLightComponent);
            SERIALIZE_COMPONENT_IF_EXISTS("DirectionalLight", Mule::DirectionalLightComponent);
            SERIALIZE_COMPONENT_IF_EXISTS("Script", Mule::ScriptComponent);

            YAML::Node childNode;
            for (auto child : e.Children())
            {
                childNode.push_back(child);
            }

            node["Children"] = childNode;

            return node;
        }

        static bool decode(const Node& node, Mule::Entity& e) {
            e = gScene->CreateEntity(node["Name"].as<std::string>(), Mule::Guid(node["Guid"].as<uint64_t>()));
            auto& transformComponent = e.GetComponent<Mule::TransformComponent>();
            transformComponent = node["Transform"].as<Mule::TransformComponent>();

            DESERIALIZE_COMPONENT_IF_EXISTS("Camera", Mule::CameraComponent);
            DESERIALIZE_COMPONENT_IF_EXISTS("EnvironmentMap", Mule::EnvironmentMapComponent);
            DESERIALIZE_COMPONENT_IF_EXISTS("Mesh", Mule::MeshComponent);
            DESERIALIZE_COMPONENT_IF_EXISTS("DirectionalLight", Mule::DirectionalLightComponent);
            DESERIALIZE_COMPONENT_IF_EXISTS("PointLight", Mule::PointLightComponent);
            convert<Mule::ScriptComponent>::gEntity = e;
            DESERIALIZE_COMPONENT_IF_EXISTS("Script", Mule::ScriptComponent);

            return true;
        }

        static Ref<Mule::Scene> gScene;
    };
}