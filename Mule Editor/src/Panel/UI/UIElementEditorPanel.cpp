#include "UIElementEditorPanel.h"

UIElementEditorPanel::UIElementEditorPanel()
	:
	IPanel("UI Element Editor")
{
}

void UIElementEditorPanel::OnAttach()
{
}

void UIElementEditorPanel::OnUIRender(float dt)
{
	if (!mIsOpen)
		return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{

	}

	ImGui::End();
}

void UIElementEditorPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}

void UIElementEditorPanel::OnEngineEvent(Ref<Mule::Event> event)
{
}
