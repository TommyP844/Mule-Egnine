 #include "SceneHierarchyPanel.h"

#include "ImGuiExtension.h"
#include "Util.h"

// Engine
#include "Mule.h"

// Submodules
#include "imgui.h"

void SceneHierarchyPanel::OnAttach()
{
}

void SceneHierarchyPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		WeakRef<Mule::Scene> scene = mEngineContext->GetScene();
		if (scene)
		{
			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					auto entity = mEngineContext->GetScene()->CreateEntity();
					mEditorContext->SetSelectedEntity(entity);
				}
				ImGui::EndPopup();
			}
			
			for(auto entity : scene->Iterate<Mule::RootComponent>())
			{
				RecurseEntities(entity);
			}

			if (mEntityToOrphan)
			{
				mEntityToOrphan.Orphan();
				mEntityToOrphan = Mule::Entity();
			}
			if (mEntityToDelete)
			{
				if (mEditorContext->GetSelectedEntity() == mEntityToDelete)
				{
					mEditorContext->SetSelectedEntity(Mule::Entity());
				}
				mEntityToDelete.Destroy();
				mEntityToDelete = Mule::Entity();
			}
		}
		else
		{
			ImGui::Text("No Scene Selected");
		}
	}
	ImGui::End();
}

void SceneHierarchyPanel::RecurseEntities(Mule::Entity e)
{
	Mule::Entity selectedEntity = mEditorContext->GetSelectedEntity();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (!e.HasChildren())
		flags |= ImGuiTreeNodeFlags_Bullet;
	if (selectedEntity == e)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (e.HasChild(selectedEntity))
		ImGui::SetNextItemOpen(true);	
	

	Mule::Guid guid = e.Guid();

	bool open = ImGui::TreeNodeEx((e.Name() + "##" + std::to_string(guid)).c_str(), flags);
	ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_ENTITY, guid, [e]() {
		ImGui::Text(e.Name().c_str());
		});
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_ENTITY, guid))
	{
		auto scene = mEngineContext->GetScene();
		auto entity = scene->GetEntityByGUID(guid);
		entity.Orphan();
		e.AddChild(entity);
	}
	ImGuiExtension::DragDropFile file;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, file))
	{
		if (file.AssetType == Mule::AssetType::Model)
		{
			fs::path filepath = file.FilePath;
			auto model = mEngineContext->GetAssetByFilepath(filepath);
			e.AddModel(model);
		}
	}
	
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		mEditorContext->SetSelectedEntity(e);

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
	std::string popupContextId = e.Name() + "_PopupContext_" + std::to_string(e.Guid());
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
