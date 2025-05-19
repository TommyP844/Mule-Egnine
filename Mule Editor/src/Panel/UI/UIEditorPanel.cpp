#include "UIEditorPanel.h"

UIEditorPanel::UIEditorPanel()
	:
	IPanel("UI Editor")
{
}

void UIEditorPanel::OnAttach()
{
}

void UIEditorPanel::OnUIRender(float dt)
{
	if (!mIsOpen)
		return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		ImGui::Text("works");
	}

	ImGui::End();
}

void UIEditorPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}

void UIEditorPanel::OnEngineEvent(Ref<Mule::Event> event)
{
}
