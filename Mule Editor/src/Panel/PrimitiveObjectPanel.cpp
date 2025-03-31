#include "PrimitiveObjectPanel.h"

#include "ImGuiExtension.h"

PrimitiveObjectPanel::PrimitiveObjectPanel()
	:
	IPanel("Primitives")
{
}

PrimitiveObjectPanel::~PrimitiveObjectPanel()
{
}

void PrimitiveObjectPanel::OnAttach()
{
}

void PrimitiveObjectPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		ImGui::Selectable("Beveled Block");
		DragDropMeshSource(MULE_BEVELED_BLOCK_MESH_HANDLE, "Beveled Block");

		ImGui::Selectable("Cube");
		DragDropMeshSource(MULE_CUBE_MESH_HANDLE, "Cube");

		ImGui::Selectable("Cone");
		DragDropMeshSource(MULE_CONE_MESH_HANDLE, "Cone");
		
		ImGui::Selectable("Plane");
		DragDropMeshSource(MULE_PLANE_MESH_HANDLE, "Torus");
		
		ImGui::Selectable("Sphere");
		DragDropMeshSource(MULE_SPHERE_MESH_HANDLE, "Sphere");
		
		ImGui::Selectable("Torus");
		DragDropMeshSource(MULE_TORUS_MESH_HANDLE, "Torus");
	}
	ImGui::End();
}

void PrimitiveObjectPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}

void PrimitiveObjectPanel::DragDropMeshSource(Mule::AssetHandle handle, const char* name)
{
	ImGuiExtension::DragDropAsset assetData;

	assetData.AssetHandle = handle;
	assetData.AssetType = Mule::AssetType::Mesh;

	ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_ASSET, assetData, [&]() {
		ImGui::Text(name);
		});
}
