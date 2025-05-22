#pragma once

#include "Graphics/UI/UIMeasurement.h"
#include "Graphics/UI/UITransform.h"
#include "Graphics/UI/Element/UIElement.h"
#include "Graphics/UI/Element/UIText.h"
#include "Graphics/UI/Element/UIButton.h"

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
            
            if (transform.Left)
            {
                const Mule::UIMeasurement& measurement = *transform.Left;
                node["Left"] = measurement;
            }
            if (transform.Top)
            {
                const Mule::UIMeasurement& measurement = *transform.Top;
                node["Top"] = measurement;
            }
            if (transform.Bottom)
            {
                const Mule::UIMeasurement& measurement = *transform.Bottom;
                node["Bottom"] = measurement;
            }
            if (transform.Right)
            {
                const Mule::UIMeasurement& measurement = *transform.Right;
                node["Right"] = measurement;
            }
            if (transform.Width)
            {
                const Mule::UIMeasurement& measurement = *transform.Width;
                node["Width"] = measurement;
            }
            if (transform.Height)
            {
                const Mule::UIMeasurement& measurement = *transform.Height;
                node["Height"] = measurement;
            }

            return node;
        }

        static bool decode(const Node& node, Mule::UITransform& transform)
        {
            if (node["Left"])
            {
                transform.Left = node["Left"].as<Mule::UIMeasurement>();
            }
            if (node["Top"])
            {
                transform.Top = node["Top"].as<Mule::UIMeasurement>();
            }
            if (node["Bottom"])
            {
                transform.Bottom = node["Bottom"].as<Mule::UIMeasurement>();
            }
            if (node["Right"])
            {
                transform.Right = node["Right"].as<Mule::UIMeasurement>();
            }
            if (node["Width"])
            {
                transform.Width = node["Width"].as<Mule::UIMeasurement>();
            }
            if (node["Height"])
            {
                transform.Height = node["Height"].as<Mule::UIMeasurement>();
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

    template<>
    struct convert<Ref<Mule::UIButton>>
    {
        static Node encode(const Ref<Mule::UIButton>& element)
        {
            Node node;

            Ref<Mule::UIElement> baseElement = element;
            node = baseElement;
            
            node["Text"] = element->GetTextElement();

            return node;
        }

        static bool decode(const Node& node, Ref<Mule::UIButton>& element)
        {
            std::string name = node["Name"].as<std::string>();
            element = MakeRef<Mule::UIButton>(name);

            Ref<Mule::UIElement> baseElem = element;
            convert<Ref<Mule::UIElement>>::decode(node, baseElem);

            Ref<Mule::UIText> textElem = node["Text"].as<Ref<Mule::UIText>>();


            return true;
        }
    };
}