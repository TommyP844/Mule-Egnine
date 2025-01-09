#include "SceneHierarchyPanel.h"

#include "Mule.h"

#include "imgui.h"


void SceneHierarchyPanel::OnUIRender()
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		WeakRef<Mule::Scene> scene = mApplicationData->GetActiveScene();
		scene->IterateRootEntities([&](Mule::Entity e) {
			ImGuiTreeNodeFlags flags = 0;
			if (!e.HasChildren())
				flags |= ImGuiTreeNodeFlags_Bullet;
			bool open = ImGui::TreeNodeEx(e.Name().c_str(), flags);

			EntityContextMenu(e);

			if(open)
			{
				for (auto child : e.Children())
				{
					RecurseEntities(child);
				}
				ImGui::TreePop();
			}
			});

		if (mEntityToOrphan) {
			mEntityToOrphan.Orphan();
			mEntityToOrphan = Mule::Entity();
		}
		if (mEntityToDelete) {
			mEntityToDelete.Destroy();
			mEntityToDelete = Mule::Entity();
		}
	}
	ImGui::End();
}

void SceneHierarchyPanel::RecurseEntities(Mule::Entity e)
{
	ImGuiTreeNodeFlags flags = 0;
	if (!e.HasChildren())
		flags |= ImGuiTreeNodeFlags_Bullet;
	bool open = ImGui::TreeNodeEx(e.Name().c_str(), flags);
	
	EntityContextMenu(e);
	
	if (open)
	{
		for (auto child : e.Children())
		{
			RecurseEntities(child);
		}
		ImGui::TreePop();
	}
}

void SceneHierarchyPanel::EntityContextMenu(Mule::Entity e)
{
	std::string popupContextId = e.Name() + "_PopupContext";
	if (ImGui::BeginPopupContextItem(popupContextId.c_str()))
	{
		ImGui::Text(e.Name().c_str());
		ImGui::Separator();
		if (e.IsChild()) 
		{
			if (ImGui::MenuItem("Orphan"))
				mEntityToOrphan = e;
		}
		if (ImGui::MenuItem("Delete"))
			mEntityToDelete = e;
		ImGui::EndPopup();
	}
}
