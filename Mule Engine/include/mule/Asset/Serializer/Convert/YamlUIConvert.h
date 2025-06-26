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
            
            node["Left"] = transform.Left;
            node["Top"] = transform.Top;
            node["Bottom"] = transform.Bottom;
            node["Right"] = transform.Right;
            node["Width"] = transform.Width;
            node["Height"] = transform.Height;
            

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
    struct convert<Mule::UIColor>
    {
        static Node encode(const Mule::UIColor& color)
        {
            Node node;

            node["Color"] = color.value;

            return node;
        }

        static bool decode(const Node& node, Mule::UIColor& color)
        {
            color.value = node["Color"].as<glm::vec4>();

            return true;
        }
    };

    template<>
    struct convert<Mule::UIPadding>
    {
        static Node encode(const Mule::UIPadding& vars)
        {
            Node node;

			node["Left"] = vars.Left;
			node["Right"] = vars.Right;
			node["Top"] = vars.Top;
			node["Bottom"] = vars.Bottom;

            return node;
        }

        static bool decode(const Node& node, Mule::UIPadding& vars)
        {
			vars.Left = node["Left"].as<Mule::UIMeasurement>();
			vars.Right = node["Right"].as<Mule::UIMeasurement>();
			vars.Top = node["Top"].as<Mule::UIMeasurement>();
			vars.Bottom = node["Bottom"].as<Mule::UIMeasurement>();

            return true;
        }
    };

    template<>
    struct convert<Mule::UIBorder>
    {
        static Node encode(const Mule::UIBorder& vars)
        {
            Node node;

            node["Thickness"] = vars.Thickness;
            node["Radius"] = vars.Radius;
            node["Color"] = vars.Color;

            return node;
        }

        static bool decode(const Node& node, Mule::UIBorder& vars)
        {
            vars.Thickness = node["Thickness"].as<Mule::UIMeasurement>();
            vars.Radius = node["Radius"].as<Mule::UIMeasurement>();
            vars.Color = node["Color"].as<Mule::UIColor>();

            return true;
        }
    };

    template<>
    struct convert<Mule::UIAnchor>
    {
        static Node encode(const Mule::UIAnchor& anchor)
        {
            Node node;

			node["TargetElement"] = anchor.TargetElement;
			node["TargetAxis"] = Mule::ToString(anchor.Target);
			node["SelfAxis"] = Mule::ToString(anchor.Self);

            return node;
        }

        static bool decode(const Node& node, Mule::UIAnchor& anchor)
        {
			anchor.TargetElement = node["TargetElement"].as<Mule::UIHandle>();
			anchor.Target = Mule::FromString<Mule::UIAnchorAxis>(node["TargetAxis"].as<std::string>());
			anchor.Self = Mule::FromString<Mule::UIAnchorAxis>(node["SelfAxis"].as<std::string>());

            return true;
        }
    };

#pragma region Styles

    template<>
    struct convert<Mule::UITextStyleVars>
    {
        static Node encode(const Mule::UITextStyleVars& vars)
        {
            Node node;


            if(vars.BackgroundColor.HasValue()) node["BackgroundColor"] = vars.BackgroundColor.GetValue();
            if(vars.Padding.HasValue())         node["Padding"] = vars.Padding.GetValue();
            if(vars.Border.HasValue())          node["Border"] = vars.Border.GetValue();
			if(vars.FontSize.HasValue())        node["FontSize"] = vars.FontSize.GetValue();
			if(vars.FontColor.HasValue())       node["FontColor"] = vars.FontColor.GetValue();
			if(vars.FontHandle.HasValue())      node["FontHandle"] = vars.FontHandle.GetValue();

            return node;
        }

        static bool decode(const Node& node, Mule::UITextStyleVars& vars)
        {
            if (node["BackgroundColor"])    vars.BackgroundColor.SetValue(node["BackgroundColor"].as<Mule::UIColor>());
			if (node["Padding"])            vars.Padding.SetValue(node["Padding"].as<Mule::UIPadding>());
			if (node["Border"])             vars.Border.SetValue(node["Border"].as<Mule::UIBorder>());
			if (node["FontSize"])           vars.FontSize.SetValue(node["FontSize"].as<float>());
			if (node["FontColor"])          vars.FontColor.SetValue(node["FontColor"].as<Mule::UIColor>());
			if (node["FontHandle"])         vars.FontHandle.SetValue(node["FontHandle"].as<Mule::AssetHandle>());

            return true;
        }
    };

    template<>
    struct convert<Mule::UIButtonStyleVars>
    {
        static Node encode(const Mule::UIButtonStyleVars& vars)
        {
            Node node;

            if (vars.BackgroundColor.HasValue()) node["BackgroundColor"] = vars.BackgroundColor.GetValue();
            if (vars.Padding.HasValue())         node["Padding"] = vars.Padding.GetValue();
            if (vars.Border.HasValue())          node["Border"] = vars.Border.GetValue();

            return node;
        }

        static bool decode(const Node& node, Mule::UIButtonStyleVars& vars)
        {
            if (node["BackgroundColor"])    vars.BackgroundColor.SetValue(node["BackgroundColor"].as<Mule::UIColor>());
            if (node["Padding"])            vars.Padding.SetValue(node["Padding"].as<Mule::UIPadding>());
            if (node["Border"])             vars.Border.SetValue(node["Border"].as<Mule::UIBorder>());

            return true;
        }
    };

#pragma endregion

#pragma region Elements

    template<>
    struct convert<Mule::UIBaseElement>
    {
        static Node encode(const Mule::UIBaseElement& element)
        {
            Node node;

			node["Name"] = element.GetName();
			node["Handle"] = element.GetHandle();
			node["Type"] = Mule::ToString(element.GetType());
			node["Transform"] = element.GetTransform();
			node["Visible"] = element.IsVisible();
			
			for (const auto& [selfAxis, anchor] : element.GetAnchors())
			{
                Node anchorNode;
                anchorNode = YAML::convert<Mule::UIAnchor>::encode(anchor);
				node["Anchors"].push_back(anchorNode);
			}
			
            return node;
        }

        static bool decode(const Node& node, Mule::UIBaseElement& element)
        {

            element.SetName(node["Name"].as<std::string>());
            element.SetHandle(node["Handle"].as<Mule::UIHandle>());
            // Mule::FromString<Mule::UIElementType>(node["Type"].as<std::string>());
            element.SetTransform(node["Transform"].as<Mule::UITransform>());
            element.SetVisible(node["Visible"].as<bool>());

            for (YAML::Node anchorNode : node["Anchors"])
            {
				Mule::UIAnchor anchor = anchorNode.as<Mule::UIAnchor>();
				element.AddAnchor(
					anchor.TargetElement,
					anchor.Target,
					anchor.Self
				);
            }

            return true;
        }
    };

    template<>
    struct convert<Mule::UIText>
    {
        static Node encode(const Mule::UIText& element)
        {
            Node node;

			node = YAML::convert<Mule::UIBaseElement>::encode(element);
			node["Text"] = element.GetText();
			node["Type"] = Mule::ToString(element.GetType());

            return node;
        }

        static bool decode(const Node& node, Mule::UIText& element)
        {
            YAML::convert<Mule::UIBaseElement>::decode(node, element);

			element.SetText(node["Text"].as<std::string>());
            // node["Type"]; Set by UIElement Derived Type constructor

            return true;
        }
    };

    template<>
    struct convert<Mule::UIButton>
    {
        static Node encode(const Mule::UIButton& element)
        {
            Node node;

            node = YAML::convert<Mule::UIBaseElement>::encode(element);
            node["Text"] = YAML::convert<Mule::UIText>::encode(*element.GetTextElement());

            return node;
        }

        static bool decode(const Node& node, Mule::UIButton& element)
        {
            YAML::convert<Mule::UIBaseElement>::decode(node, element);

			YAML::convert<Mule::UIText>::decode(node["Text"], *element.GetTextElement());
            // node["Type"]; Set by UIElement Derived Type constructor

            return true;
        }
    };

#pragma endregion

}