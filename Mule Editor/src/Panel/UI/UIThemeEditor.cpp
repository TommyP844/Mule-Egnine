#include "UIThemeEditor.h"

#include "ImGuiExtension.h"
#include "Event/EditUIThemeEvent.h"

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

		for (uint32_t i = 0; i < static_cast<uint32_t>(Mule::UIStyleKey::STYLE_KEY_MAX); i++)
		{
			Mule::UIStyleKey key = static_cast<Mule::UIStyleKey>(i);
			Mule::UIStyleKeyDataType dataType = Mule::GetUIStyleKeyDataType(key);
			std::string styleName = Mule::GetUIStyleKeyName(key);
			std::string hiddenStyleName = "##" + styleName;

			ImGui::Text(styleName.c_str());
			ImGui::SameLine(150.f);
			ImGui::PushItemWidth(250.f);

			ImGui::PushID(i);

			switch (dataType)
			{
			case Mule::UIStyleKeyDataType::Bool:
			{
				auto val = mTheme->GetValue<bool>(key);
				if (ImGui::Checkbox(hiddenStyleName.c_str(), &val))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::Integer:
			{
				auto val = mTheme->GetValue<int>(key);
				if (ImGui::DragInt(hiddenStyleName.c_str(), &val))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::Float:
			{
				auto val = mTheme->GetValue<float>(key);
				if (ImGui::DragFloat(hiddenStyleName.c_str(), &val))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::Vec2:
			{
				auto val = mTheme->GetValue<glm::vec2>(key);
				if (ImGui::DragFloat2(hiddenStyleName.c_str(), &val[0]))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::Vec3:
			{
				auto val = mTheme->GetValue<glm::vec3>(key);
				if (ImGui::DragFloat3(hiddenStyleName.c_str(), &val[0]))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::Color:
			{
				auto val = mTheme->GetValue<glm::vec4>(key);
				if (ImGui::ColorEdit4(hiddenStyleName.c_str(), &val[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
				{
					mTheme->SetValue(key, val);
				}
				break;
			}
			case Mule::UIStyleKeyDataType::Ivec2:
			{
				auto val = mTheme->GetValue<glm::ivec2>(key);
				if (ImGui::DragInt2(hiddenStyleName.c_str(), &val[0]))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::IVec3:
			{
				auto val = mTheme->GetValue<glm::ivec3>(key);
				if (ImGui::DragInt3(hiddenStyleName.c_str(), &val[0]))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::Ivec4:
			{
				auto val = mTheme->GetValue<glm::ivec4>(key);
				if (ImGui::DragInt4(hiddenStyleName.c_str(), &val[0]))
				{
					mTheme->SetValue(key, val);
					mIsModified = true;
				}
				break;
			}
			case Mule::UIStyleKeyDataType::AssetHandle:
			{
				auto assetManager = mEngineContext->GetAssetManager();
				auto fontHandle = mTheme->GetValue<Mule::AssetHandle>(key);
				auto font = assetManager->Get<Mule::UIFont>(fontHandle);

				std::string fontName = "(Null)";

				if (font)
					fontName = font->Name();

				ImGui::Text(fontName.c_str());
				ImGuiExtension::DragDropFile ddf;
				if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
				{
					mTheme->SetValue(key, ddf.AssetHandle);
					mIsModified = true;
				}
			}
			break;
			}

			ImGui::PopID();

			ImGui::Separator();
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
