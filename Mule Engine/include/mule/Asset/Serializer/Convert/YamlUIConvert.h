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

}