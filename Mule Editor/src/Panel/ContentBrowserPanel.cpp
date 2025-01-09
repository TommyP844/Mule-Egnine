#include "ContentBrowserPanel.h"

#include <imgui.h>

void ContentBrowserPanel::OnUIRender()
{
	if (!mIsOpen) return;
	ImGui::Begin(mName.c_str(), &mIsOpen);

	ImGui::End();
}
