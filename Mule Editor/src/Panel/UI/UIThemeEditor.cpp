#include "UIThemeEditor.h"

#include "ImGuiExtension.h"
#include "Event/EditUIThemeEvent.h"
#include "UIEditorDisplay.h"

UIThemeEditor::UIThemeEditor()
	:
	IPanel("UI Theme Editor"),
	mTheme(nullptr),
	mIsModified(false)
{
}

UIThemeEditor::~UIThemeEditor()
{
}

void UIThemeEditor::OnAttach()
{
}

void UIThemeEditor::OnUIRender(float dt)
{
	if (!mIsOpen)
		return;

	if (ImGui::Begin(mName.c_str()))
	{
		if (!mTheme)
		{
			ImGui::End();
			return;
		}

		fs::path assetPath = mEditorContext->GetAssetsPath();
		std::string name = mTheme->Name();
		std::string filepath = mTheme->FilePath().lexically_relative(assetPath).string();
		uint64_t handle = mTheme->Handle();

		ImGui::Text("Name: %s", name.c_str());
		ImGui::Text("Filepath: %s", filepath.c_str());
		ImGui::Text("Handle: %llu", handle);

		ImGui::BeginDisabled(!mIsModified);
		if (ImGui::Button("Save"))
		{
			mEngineContext->GetAssetManager()->Save<Mule::UITheme>(mTheme->Handle());
			mIsModified = false;
		}
		ImGui::EndDisabled();

		ImGui::SeparatorText("Theme Values");

		if (ImGui::BeginTabBar("ThemeStates"))
		{
			for (auto state : Mule::AllUIStates)
			{
				std::string stateName = Mule::ToString(state);
				if (ImGui::BeginTabItem(stateName.c_str()))
				{
					if (ImGui::CollapsingHeader("Button Style"))
					{
						mIsModified |= DisplayButtonStyleEditor(mTheme->ButtonStyle, state);
						ImGui::SeparatorText("Button Text Style");
						mIsModified |= DisplayTextStyleEditor(mTheme->ButtonStyle->TextStyle.GetValue(), state);
					}

					if (ImGui::CollapsingHeader("Text Style"))
						mIsModified |= DisplayTextStyleEditor(mTheme->TextStyle, state);

					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void UIThemeEditor::OnEditorEvent(Ref<IEditorEvent> event)
{
	switch (event->GetEventType())
	{
	case EditorEventType::EditUITheme:
	{
		auto assetManager = mEngineContext->GetAssetManager();
		Ref<EditUIThemeEvent> themeEvent = event;
		mTheme = assetManager->Get<Mule::UITheme>(themeEvent->GetUIThemeHandle());
	}
		break;
	}
}

void UIThemeEditor::OnEngineEvent(Ref<Mule::Event> event)
{
}

void UIThemeEditor::SetTheme(WeakRef<Mule::UITheme> theme)
{
	mTheme = theme;
	mIsModified = false;
}
