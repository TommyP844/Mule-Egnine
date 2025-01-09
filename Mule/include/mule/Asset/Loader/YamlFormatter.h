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

#define SERIALIZE_COMPONENT_IF_EXISTS(name, x) if(e.HasComponent<x>()) node[name] = e.GetComponent<x>();

    template<>
    struct convert<Mule::Entity> {
        static Node encode(const Mule::Entity& e) {
            Node node;

            node["id"] = (int)e.ID();
            node["name"] = e.Name();
            node["guid"] = (size_t)e.Guid();

            node["transform"] = e.GetComponent<Mule::TransformComponent>();

            SERIALIZE_COMPONENT_IF_EXISTS("camera", Mule::CameraComponent);

            YAML::Node childNode;
            for (auto child : e.Children())
            {
                childNode.push_back(child);
            }

            node["children"] = childNode;
            
            return node;
        }

        static bool decode(const Node& node, Mule::Entity& e) {
            

            return true;
        }
    };

#pragma region Components

    template<>
    struct convert<Mule::TransformComponent> {
        static Node encode(const Mule::TransformComponent& transform) {
            Node node;

            node["translation"] = transform.Translation;
            node["rotation"] = transform.Rotation;
            node["scale"] = transform.Scale;

            return node;
        }

        static bool decode(const Node& node, Mule::TransformComponent& transform) {


            return true;
        }
    };

    template<>
    struct convert<Mule::CameraComponent> {
        static Node encode(const Mule::CameraComponent& camera) {
            Node node;

            node["active"] = camera.Active;

            return node;
        }

        static bool decode(const Node& node, Mule::CameraComponent& camera) {


            return true;
        }
    };

#pragma endregion
}