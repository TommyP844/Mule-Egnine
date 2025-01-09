#include "ComponentPanel.h"

#include "imgui.h"

void ComponentPanel::OnUIRender()
{
	if (!mIsOpen) return;
	ImGui::Begin(mName.c_str(), &mIsOpen);


	ImGui::End();
}
