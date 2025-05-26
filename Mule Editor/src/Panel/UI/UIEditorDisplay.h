#pragma once

#include "Mule.h"
#include <imgui.h>

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
	else
	{
		ImGui::Text("Unsupported type");
	}

	ImGui::PopID();

	return ret;
}

static bool DisplayButtonStyleEditor(WeakRef<Mule::UIButtonStyle> style, Mule::UIElementState state)
{
	bool modified = false;

	Mule::UIButtonStyleVars& vars = style->GetStateVars(state);

	modified |= DisplayStyleVar("Background Color", vars.BackgroundColor);
	modified |= DisplayStyleVar("Foreground Color", vars.ForegroundColor);
	modified |= DisplayStyleVar("Padding", vars.Padding);
	modified |= DisplayStyleVar("Font Handle", vars.FontHandle);

	return modified;
}

static bool DisplayTextStyleEditor(WeakRef<Mule::UITextStyle> style, Mule::UIElementState state)
{
	bool modified = false;

	Mule::UITextStyleVars& vars = style->GetStateVars(state);

	modified |= DisplayStyleVar("Background Color", vars.BackgroundColor);
	modified |= DisplayStyleVar("Foreground Color", vars.ForegroundColor);
	modified |= DisplayStyleVar("Font Handle", vars.FontHandle);
	modified |= DisplayStyleVar("Font Size", vars.FontSize);

	return modified;
}