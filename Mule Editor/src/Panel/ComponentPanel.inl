#pragma once

template<typename T>
inline void ComponentPanel::DisplayComponent(const char* name,Mule::Entity e, std::function<void(T&)> func)
{
	if (!e.HasComponent<T>()) return;
	bool open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_CollapsingHeader);

	if (ImGui::BeginPopupContextItem(name))
	{
		if (ImGui::MenuItem("Delete"))
		{
			e.RemoveComponent<T>();
			ImGui::EndPopup();
			return;
		}
		ImGui::EndPopup();
	}

	if (open)
	{
		if (ImGui::BeginTable("name", 2, ImGuiTableFlags_NoClip | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV, { ImGui::GetContentRegionAvail().x, 0.f }))
		{
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Data");
			func(e.GetComponent<T>());
			ImGui::EndTable();
		}
	}
}