#include "UIStyleEditorPanel.h"

#include "Event/EditUIStyleEvent.h"

UIStyleEditorPanel::UIStyleEditorPanel()
	:
	IPanel("UI Style Editor"),
	mIsModified(false)
{
}

void UIStyleEditorPanel::OnAttach()
{
}

void UIStyleEditorPanel::OnUIRender(float dt)
{
	if (!mIsOpen)
		return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		if (!mStyle)
		{
			ImGui::End();
			return;
		}

		fs::path assetPath = mEditorContext->GetAssetsPath();
		std::string name = mStyle->Name();
		std::string filepath = mStyle->FilePath().lexically_relative(assetPath).string();
		uint64_t handle = mStyle->Handle();

		ImGui::Text("Name: %s", name.c_str());
		ImGui::Text("Filepath: %s", filepath.c_str());
		ImGui::Text("Handle: %llu", handle);

		ImGui::BeginDisabled(!mIsModified);
		if (ImGui::Button("Save"))
		{
			mEngineContext->GetAssetManager()->Save<Mule::UIStyle>(mStyle->Handle());
			mIsModified = false;
		}
		ImGui::EndDisabled();

		ImGui::SeparatorText("Style Values");

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

			if (!mStyle->HasValue(key))
			{
				if (ImGui::Button("Add"))
				{
					switch (dataType)
					{
					case Mule::UIStyleKeyDataType::Bool:	mStyle->SetValue(key, false); break;
					case Mule::UIStyleKeyDataType::Integer: mStyle->SetValue(key, 0); break;
					case Mule::UIStyleKeyDataType::Float:	mStyle->SetValue(key, 0.f); break;
					case Mule::UIStyleKeyDataType::Vec2:	mStyle->SetValue(key, glm::vec2(0.f)); break;
					case Mule::UIStyleKeyDataType::Vec3:	mStyle->SetValue(key, glm::vec3(0.f)); break;
					case Mule::UIStyleKeyDataType::Color:	mStyle->SetValue(key, glm::vec4(1.f)); break;
					case Mule::UIStyleKeyDataType::Ivec2:	mStyle->SetValue(key, glm::ivec2(0)); break;
					case Mule::UIStyleKeyDataType::IVec3:	mStyle->SetValue(key, glm::ivec3(0)); break;
					case Mule::UIStyleKeyDataType::Ivec4:	mStyle->SetValue(key, glm::ivec4(0)); break;
					}

					mIsModified = true;
				}
			}
			else
			{
				switch (dataType)
				{
				case Mule::UIStyleKeyDataType::Bool:
				{
					auto val = mStyle->GetValue<bool>(key);
					if (ImGui::Checkbox(hiddenStyleName.c_str(), &val))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				case Mule::UIStyleKeyDataType::Integer:
				{
					auto val = mStyle->GetValue<int>(key);
					if (ImGui::DragInt(hiddenStyleName.c_str(), &val))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				case Mule::UIStyleKeyDataType::Float:
				{
					auto val = mStyle->GetValue<float>(key);
					if (ImGui::DragFloat(hiddenStyleName.c_str(), &val))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				case Mule::UIStyleKeyDataType::Vec2:
				{
					auto val = mStyle->GetValue<glm::vec2>(key);
					if (ImGui::DragFloat2(hiddenStyleName.c_str(), &val[0]))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				case Mule::UIStyleKeyDataType::Vec3:
				{
					auto val = mStyle->GetValue<glm::vec3>(key);
					if (ImGui::DragFloat3(hiddenStyleName.c_str(), &val[0]))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				case Mule::UIStyleKeyDataType::Color:
				{
					auto val = mStyle->GetValue<glm::vec4>(key);
					if (ImGui::ColorEdit4(hiddenStyleName.c_str(), &val[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
					{
						mStyle->SetValue(key, val);
					}
					break;
				}
				case Mule::UIStyleKeyDataType::Ivec2:
				{
					auto val = mStyle->GetValue<glm::ivec2>(key);
					if (ImGui::DragInt2(hiddenStyleName.c_str(), &val[0]))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				case Mule::UIStyleKeyDataType::IVec3:
				{
					auto val = mStyle->GetValue<glm::ivec3>(key);
					if (ImGui::DragInt3(hiddenStyleName.c_str(), &val[0]))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				case Mule::UIStyleKeyDataType::Ivec4:
				{
					auto val = mStyle->GetValue<glm::ivec4>(key);
					if (ImGui::DragInt4(hiddenStyleName.c_str(), &val[0]))
					{
						mStyle->SetValue(key, val);
						mIsModified = true;
					}
					break;
				}
				}
			}

			ImGui::PopID();

			ImGui::Separator();
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
		WeakRef<EditUIStyleEvent> editUiStyleEvent = event;
		auto assetManager = mEngineContext->GetAssetManager();
		mStyle = assetManager->Get<Mule::UIStyle>(editUiStyleEvent->GetUIStyleHandle());
		Open();
	}
	break;
	}
}

void UIStyleEditorPanel::OnEngineEvent(Ref<Mule::Event> event)
{
}

void UIStyleEditorPanel::SetStyle(Ref<Mule::UIStyle> style)
{
	mStyle = style;
	mIsModified = false;
}
