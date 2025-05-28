#pragma once

#include "Mule.h"
#include <imgui.h>

static bool DisplayUIMeasurement(const char* label, Mule::UIMeasurement& measurement, float offset = 75.f)
{
	bool modified = false;

	ImGui::PushID(label);
	ImGui::Text(label);
	ImGui::SameLine(offset);
	ImGui::PushItemWidth(100.f);
	if (ImGui::DragFloat("##MeasurementValue", &measurement.Value, 1.f, 0.f, 0.f, "%.1f"))
	{
		modified = true;
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();

	const char* options[] = {
		"px",
		"%",
		"Auto"
	};

	Mule::UIUnitType currentType = measurement.GetUnitType();
	const char* selectedOption = options[static_cast<uint32_t>(currentType)];

	ImGui::PushItemWidth(50.f);
	if (ImGui::BeginCombo("##Combo", selectedOption))
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(Mule::UIUnitType::MAX_UNIT_TYPE); i++)
		{
			Mule::UIUnitType type = static_cast<Mule::UIUnitType>(i);
			bool selected = (type == currentType);

			if (ImGui::Selectable(options[i], selected))
			{
				measurement.SetUnitType(static_cast<Mule::UIUnitType>(i));
				modified = true;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
	ImGui::PopID();

	return modified;
}

template<typename T>
static bool DisplayStyleVar(const char* name, Mule::StyleVariable<T>& var)
{
	bool ret = false;
	ImGui::PushID(name);

	const float labelOffset = 150.f;

	ImGui::Text(name);
	ImGui::SameLine(labelOffset);
	if (!var.HasValue())
	{
		if (ImGui::Button("Set Default"))
		{
			var.SetValue(T());
			ret = true;
		}
		ImGui::PopID();
		return ret;
	}

	if constexpr (std::is_same_v<T, Mule::UIColor>)
	{
		Mule::UIColor value = var.GetValue();
		if (ImGui::ColorEdit4("##Value", &value[0]))
		{
			var.SetValue(value);
			ret = true;
		}
	}
	else if constexpr (std::is_same_v<T, glm::vec4>)
	{
		glm::vec4 value = var.GetValue();
		if (ImGui::DragFloat4("##Value", &value[0]))
		{
			var.SetValue(value);
			ret = true;
		}
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		float value = var.GetValue();
		if (ImGui::DragFloat("##Value", &value))
		{
			var.SetValue(value);
			ret = true;
		}
	}
	else if constexpr (std::is_same_v<T, Mule::AssetHandle>)
	{
		ImGui::Text("AssetHandle");
	}
	else if constexpr (std::is_same_v<T, Mule::UIPadding>)
	{
		Mule::UIPadding& value = var.GetValue();
		
		ImGui::NewLine();
		
		ret |= DisplayUIMeasurement("Left", value.Left, labelOffset);
		ret |= DisplayUIMeasurement("Right", value.Right, labelOffset);
		ret |= DisplayUIMeasurement("Top", value.Top, labelOffset);
		ret |= DisplayUIMeasurement("Bottom", value.Bottom, labelOffset);
	}
	else if constexpr (std::is_same_v<T, Mule::UIBorder>)
	{
		Mule::UIBorder& value = var.GetValue();
		
		ImGui::NewLine();
		
		ret |= DisplayUIMeasurement("Thickness", value.Thickness, labelOffset);
		ret |= DisplayUIMeasurement("Radius", value.Radius, labelOffset);

		ImGui::Text("Color");
		ImGui::SameLine(labelOffset);
		Mule::UIColor& color = value.Color;
		if (ImGui::ColorEdit4("##Value", &color[0]))
		{
			ret = true;
		}
	}
	else
	{
		ImGui::Text("Unsupported type");
	}

	ImGui::PopID();

	ImGui::Separator();

	return ret;
}

#define DISPALY_BASE_STYLE_VARS(vars) \
	modified |= DisplayStyleVar("Background Color", vars.BackgroundColor); \
	modified |= DisplayStyleVar("Padding", vars.Padding); \
	modified |= DisplayStyleVar("Border", vars.Border); \

static bool DisplayButtonStyleEditor(WeakRef<Mule::UIButtonStyle> style, Mule::UIElementState state)
{
	bool modified = false;

	Mule::UIButtonStyleVars& vars = style->GetStateVars(state);

	ImGui::PushID("ButtonStyleEditor");
	DISPALY_BASE_STYLE_VARS(vars);
	ImGui::PopID();

	return modified;
}

static bool DisplayTextStyleEditor(WeakRef<Mule::UITextStyle> style, Mule::UIElementState state)
{
	bool modified = false;

	Mule::UITextStyleVars& vars = style->GetStateVars(state);

	ImGui::PushID("TextStyleEditor");
	DISPALY_BASE_STYLE_VARS(vars);
	ImGui::PopID();

	modified |= DisplayStyleVar("Font Handle", vars.FontHandle);
	modified |= DisplayStyleVar("Font Size", vars.FontSize);

	return modified;
}

#pragma region Element Editors

static bool DisplayTextElementEditor(WeakRef<Mule::UIText> element)
{
	bool modified = false;

	static char buffer[1024] = { 0 };
	std::string text = element->GetText();
	memset(buffer, 0, 1024);
	memcpy(buffer, text.data(), text.size());
	ImGui::Text("Text");
	ImGui::SameLine();
	if (ImGui::InputTextMultiline("##Text", buffer, 1024, { 250, 150 }))
	{
		element->SetText(buffer);
		modified = true;
	}

	return modified;
}

static bool DisplayButtonElementEditor(WeakRef<Mule::UIButton> element)
{
	bool modified = false;

	ImGui::SeparatorText("Horizontal Text Align");

	Ref<Mule::UIText> textElement = element->GetTextElement();

	if (ImGui::Button("Clear Anchors"))
		textElement->RemoveAllAnchors();

	if (ImGui::Button("Left"))
	{
		textElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Left, Mule::UIAnchorAxis::Left);
		textElement->RemoveAnchor(Mule::UIAnchorAxis::Right);
		textElement->RemoveAnchor(Mule::UIAnchorAxis::CenterHorizontal);
	}

	ImGui::SameLine();

	if (ImGui::Button("Center"))
	{
		textElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::CenterHorizontal, Mule::UIAnchorAxis::CenterHorizontal);
		textElement->RemoveAnchor(Mule::UIAnchorAxis::Right);
		textElement->RemoveAnchor(Mule::UIAnchorAxis::Left);
	}

	ImGui::SameLine();

	if (ImGui::Button("Right"))
	{
		textElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Right, Mule::UIAnchorAxis::Right);
		textElement->RemoveAnchor(Mule::UIAnchorAxis::Left);
		textElement->RemoveAnchor(Mule::UIAnchorAxis::CenterHorizontal);
	}

	if (ImGui::CollapsingHeader("Text"))
	{
		modified |= DisplayTextElementEditor(textElement);
	}

	return modified;
}


#pragma endregion