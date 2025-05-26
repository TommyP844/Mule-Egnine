#include "UIStyleEditorPanel.h"

#include "Event/EditUIStyleEvent.h"
#include "UIEditorDisplay.h"

UIStyleEditorPanel::UIStyleEditorPanel()
	:
	IPanel("UI Style Editor"),
	mIsModified(false)
{
}

void UIStyleEditorPanel::OnAttach()
{
	mTempStyle = MakeRef<Mule::UIButtonStyle>();
	mStyle = mTempStyle;
}

void UIStyleEditorPanel::OnUIRender(float dt)
{
	if (!mIsOpen || !mStyle)
		return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		Mule::UIElementType type = mStyle->GetElementType();
		std::string styleTypeStr = Mule::ToString(type);

		ImGui::Text("Style Type: %s", styleTypeStr.c_str());

		float width = ImGui::GetContentRegionAvail().x;
		width -= ImGui::CalcTextSize("Save").x + ImGui::GetStyle().FramePadding.x * 2.f;
		ImGui::SameLine(width);

		ImGui::BeginDisabled(!mIsModified);

		if (ImGui::Button("Save"))
		{
			//mEngineContext->GetAssetManager()->Save<Mule::UIBaseStyle>(handle, mStyle);
			mIsModified = false;
		}

		ImGui::EndDisabled();

		ImGui::Separator();

		if (ImGui::BeginTabBar("StateTabBar"))
		{
			for (auto state : Mule::AllUIStates)
			{
				std::string tabName = Mule::ToString(state);
				if (ImGui::BeginTabItem(tabName.c_str()))
				{
					switch (type)
					{
					case Mule::UIElementType::UIButton:
						mIsModified |= DisplayButtonStyleEditor(mStyle, state);
						break;
					}

					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void UIStyleEditorPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
	switch (event->GetEventType())
	{
	case EditorEventType::EditUIStyle:
	{
	}
	break;
	}
}

void UIStyleEditorPanel::OnEngineEvent(Ref<Mule::Event> event)
{
}
