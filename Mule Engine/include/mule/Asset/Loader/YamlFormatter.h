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
            node["NearPlane"] = camera.Camera->GetNearPlane();
            node["FarPlane"] = camera.Camera->GetFarPlane();
            node["FOV"] = camera.Camera->GetFOVDegrees();
            node["Yaw"] = camera.Camera->GetYaw();
            node["Pitch"] = camera.Camera->GetPitch();
            node["WorldUp"] = camera.Camera->GetWorldUp();
            node["Position"] = camera.Camera->GetPosition();

            return node;
        }

        static bool decode(const Node& node, Mule::CameraComponent& camera) {

            camera.Active = node["Active"].as<bool>();
            
            camera.Camera = MakeRef<Mule::Camera>();
            camera.Camera->SetNearPlane(node["NearPlane"].as<float>());
            camera.Camera->SetFarPlane(node["FarPlane"].as<float>());
            camera.Camera->SetFOVDegrees(node["FOV"].as<float>());
            camera.Camera->SetYaw(node["Yaw"].as<float>());
            camera.Camera->SetPitch(node["Pitch"].as<float>());
            camera.Camera->SetWorldUp(node["WorldUp"].as<glm::vec3>());
            camera.Camera->SetPosition(node["Position"].as<glm::vec3>());

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

        static WeakRef<Mule::ScriptContext> gScriptContext;
    };

    template<>
    struct convert<Mule::RigidBody3DComponent> {
        static Node encode(const Mule::RigidBody3DComponent& rigidBody) {
            Node node;

            node["Mass"] = rigidBody.Mass;
            node["BodyType"] = (uint32_t)rigidBody.BodyType;

            return node;
        }

        static bool decode(const Node& node, Mule::RigidBody3DComponent& rigidBody) {

            rigidBody.Mass = node["Mass"].as<float>();
            rigidBody.BodyType = (Mule::BodyType)node["BodyType"].as<uint32_t>();

            return true;
        }
    };

    template<>
    struct convert<Mule::SphereColliderComponent> {
        static Node encode(const Mule::SphereColliderComponent& collider) {
            Node node;

            node["Trigger"] = collider.Trigger;
            node["Offset"] = collider.Offset;
            node["Radius"] = collider.Radius;

            return node;
        }

        static bool decode(const Node& node, Mule::SphereColliderComponent& collider) {

            collider.Trigger = node["Trigger"].as<bool>();
            collider.Offset = node["Offset"].as<glm::vec3>();
            collider.Radius = node["Radius"].as<float>();

            return true;
        }
    };

    template<>
    struct convert<Mule::BoxColliderComponent> {
        static Node encode(const Mule::BoxColliderComponent& collider) {
            Node node;

            node["Trigger"] = collider.Trigger;
            node["Offset"] = collider.Offset;
            node["Extent"] = collider.Extent;

            return node;
        }

        static bool decode(const Node& node, Mule::BoxColliderComponent& collider) {

            collider.Trigger = node["Trigger"].as<bool>();
            collider.Offset = node["Offset"].as<glm::vec3>();
            collider.Extent = node["Extent"].as<glm::vec3>();

            return true;
        }
    };

#pragma endregion
}