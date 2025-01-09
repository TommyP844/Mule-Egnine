#include "SceneViewPanel.h"

#include "imgui.h"

void SceneViewPanel::OnUIRender()
{
	if (!mIsOpen) return;
	ImGui::Begin(mName.c_str(), &mIsOpen);

	ImGui::End();
}
