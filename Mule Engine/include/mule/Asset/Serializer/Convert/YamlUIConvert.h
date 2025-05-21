#pragma once

#include "Graphics/UI/UIMeasurement.h"
#include "Graphics/UI/UITransform.h"
#include "Graphics/UI/Element/UIElement.h"
#include "Graphics/UI/Element/UIText.h"

#include <yaml-cpp/yaml.h>

namespace YAML 
{
    template<>
    struct convert<Mule::UIHandle>
    {
        static Node encode(const Mule::UIHandle& handle)
        {
            Node node;
            node = handle.Handle;

            return node;
        }

        static bool decode(const Node& node, Mule::UIHandle& handle)
        {
            handle.Handle = node.as<uint64_t>();

            return true;
        }
    };

    template<>
    struct convert<Mule::UIMeasurement>
    {
        static Node encode(const Mule::UIMeasurement& measurement)
        {
            Node node;
            node["Value"] = measurement.Value;
            node["Type"] = Mule::GetUIUnitTypeString(measurement.GetUnitType());

            return node;
        }

        static bool decode(const Node& node, Mule::UIMeasurement& measurement)
        {
            measurement.Init(node["Value"].as<float>(), Mule::GetUIUnitTypeFromString(node["Type"].as<std::string>()));

            return true;
        }
    };

    template<>
    struct convert<Mule::UITransform>
    {
        static Node encode(const Mule::UITransform& transform)
        {
            Node node;
            
            if (transform.GetLeft())
            {
                const Mule::UIMeasurement& measurement = *transform.GetLeft();
                node["Left"] = measurement;
            }
            if (transform.GetTop())
            {
                const Mule::UIMeasurement& measurement = *transform.GetTop();
                node["Top"] = measurement;
            }
            if (transform.GetBottom())
            {
                const Mule::UIMeasurement& measurement = *transform.GetBottom();
                node["Bottom"] = measurement;
            }
            if (transform.GetRight())
            {
                const Mule::UIMeasurement& measurement = *transform.GetRight();
                node["Right"] = measurement;
            }
            if (transform.GetWidth())
            {
                const Mule::UIMeasurement& measurement = *transform.GetWidth();
                node["Width"] = measurement;
            }
            if (transform.GetHeight())
            {
                const Mule::UIMeasurement& measurement = *transform.GetHeight();
                node["Height"] = measurement;
            }

            return node;
        }

        static bool decode(const Node& node, Mule::UITransform& transform)
        {
            if (node["Left"])
            {
                transform.SetLeft(node["Left"].as<Mule::UIMeasurement>());
            }
            if (node["Top"])
            {
                transform.SetTop(node["Top"].as<Mule::UIMeasurement>());
            }
            if (node["Bottom"])
            {
                transform.SetBottom(node["Bottom"].as<Mule::UIMeasurement>());
            }
            if (node["Right"])
            {
                transform.SetRight(node["Right"].as<Mule::UIMeasurement>());
            }
            if (node["Width"])
            {
                transform.SetWidth(node["Width"].as<Mule::UIMeasurement>());
            }
            if (node["Height"])
            {
                transform.SetHeight(node["Height"].as<Mule::UIMeasurement>());
            }

            return true;
        }
    };

    template<>
    struct convert<Ref<Mule::UIElement>>
    {
        static Node encode(const Ref<Mule::UIElement>& element)
        {
            Node node;

            node["Handle"] = element->GetHandle();
            node["Type"] = GetUIElementNameFromType(element->GetType());
            const Mule::UITransform& transform = element->GetTransform();
            node["Transform"] = transform;
            node["Name"] = element->GetName();

            return node;
        }

        static bool decode(const Node& node, Ref<Mule::UIElement>& element)
        {
            Mule::UIHandle handle = node["Handle"].as<Mule::UIHandle>();
            Mule::UIElementType type = Mule::GetUIElementTypeFromString(node["Type"].as<std::string>());
            Mule::UITransform transform = node["Transform"].as<Mule::UITransform>();
            std::string name = node["Name"].as<std::string>();

            element->SetTransform(transform);        
            
            return true;
        }
    };

    template<>
    struct convert<Ref<Mule::UIText>>
    {
        static Node encode(const Ref<Mule::UIText>& element)
        {
            Node node;

            Ref<Mule::UIElement> baseElement = element;
            node = baseElement;
            node["Text"] = element->GetText();

            return node;
        }

        static bool decode(const Node& node, Ref<Mule::UIText>& element)
        {
            std::string name = node["Name"].as<std::string>();
            element = MakeRef<Mule::UIText>(name);

            Ref<Mule::UIElement> baseElem = element;
            convert<Ref<Mule::UIElement>>::decode(node, baseElem);

            std::string text = node["Text"].as<std::string>();
            element->SetText(text);

            return true;
        }
    };
}