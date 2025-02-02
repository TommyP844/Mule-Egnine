#include "SceneViewPanel.h"

#include "imgui.h"
#include "ImGuizmo.h"

SceneViewPanel::SceneViewPanel()
	: 
	IPanel("Scene View"),
	mWidgetTranslation(false),
	mWidgetRotation(false),
	mWidgetScale(false)
{
}

void SceneViewPanel::OnAttach()
{
	mWidth = 0;
	mHeight = 0;
}

void SceneViewPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		Ref<Mule::Scene> scene = mEngineContext->GetScene();
		if (scene)
		{
			Ref<Mule::SceneRenderer> sceneRenderer = mEngineContext->GetSceneRenderer();

			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			ImVec2 region = ImGui::GetContentRegionAvail();
			if (region.x != mWidth || region.y != mHeight)
			{
				mWidth = region.x;
				mHeight = region.y;
				sceneRenderer->Resize(mWidth, mHeight);
				mEditorState->EditorRenderSettings.EditorCamera.SetAspectRatio(mWidth / mHeight);
			}

			Mule::Camera& camera = mEditorState->EditorRenderSettings.EditorCamera;
			glm::vec3 cameraPosition = camera.GetPosition();

			Ref<Mule::FrameBuffer> frameBuffer = sceneRenderer->GetCurrentFrameBuffer();
			WeakRef<Mule::Texture2D> texture = frameBuffer->GetColorAttachment(0);
			ImGui::Image(texture->GetImGuiID(), region);

			// TODO: move to OnEvent
			
			if (ImGui::IsWindowFocused())
			{
				const float speed = 0.5f;
				if (ImGui::IsKeyDown(ImGuiKey_W))
				{
					cameraPosition += camera.GetForwardDir() * speed;
				}
				if (ImGui::IsKeyDown(ImGuiKey_A))
				{
					cameraPosition -= camera.GetRightDir() * speed;
				}
				if (ImGui::IsKeyDown(ImGuiKey_S))
				{
					cameraPosition -= camera.GetForwardDir() * speed;
				}
				if (ImGui::IsKeyDown(ImGuiKey_D))
				{
					cameraPosition += camera.GetRightDir() * speed;
				}
				if (ImGui::IsKeyDown(ImGuiKey_R))
				{
					cameraPosition += camera.GetWorldUp() * speed;
				}
				if (ImGui::IsKeyDown(ImGuiKey_F))
				{
					cameraPosition -= camera.GetWorldUp() * speed;
				}
				camera.SetPosition(cameraPosition);


				static bool leftPressed = false;
				static bool justPressed = false;
				static glm::vec2 cursorPos = { 0, 0 };
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && leftPressed == false)
				{
					leftPressed = true;
					justPressed = true;
					//cursorPos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					leftPressed = false;
				}

				if (leftPressed)
				{
					glm::vec2 curPos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y};
					glm::vec2 delta = curPos - cursorPos;
					if (justPressed)
					{
						delta = { 0, 0 };
						justPressed = false;
					}
					cursorPos = curPos;
					camera.Rotate(delta.x * 0.1f, delta.y * 0.1f);
				}

				glm::mat4 view = camera.GetView();
				glm::mat4 proj = camera.GetProj();
				glm::mat4 identity = glm::identity<glm::mat4>();
				ImGuizmo::DrawGrid(&view[0][0], &proj[0][0], &identity[0][0], 50.f);

				if (mEditorState->SelectedEntity)
				{
					if (ImGui::IsKeyPressed(ImGuiKey_T))
						mWidgetTranslation = !mWidgetTranslation;
					if (ImGui::IsKeyPressed(ImGuiKey_Y))
						mWidgetRotation = !mWidgetRotation;
					if (ImGui::IsKeyPressed(ImGuiKey_U))
						mWidgetScale = !mWidgetScale;

					uint32_t operation = 0;
					if (mWidgetTranslation) operation |= ImGuizmo::OPERATION::TRANSLATE;
					if (mWidgetRotation) operation |= ImGuizmo::OPERATION::ROTATE;
					if (mWidgetScale) operation |= ImGuizmo::OPERATION::SCALE;

					if (operation != 0)
					{
						ImGuizmo::SetOrthographic(false);  // Set to true if using an orthographic camera
						ImGuizmo::SetDrawlist();
						ImGuizmo::SetRect(cursorPos.x, cursorPos.y, mWidth, mHeight);

						Mule::TransformComponent& transform = mEditorState->SelectedEntity.GetTransformComponent();

						
						glm::mat4 transformMatrix = transform.TRS();
						if (ImGuizmo::Manipulate(&view[0][0], &proj[0][0], (ImGuizmo::OPERATION)operation, ImGuizmo::MODE::WORLD, &transformMatrix[0][0]))
						{
							ImGuizmo::DecomposeMatrixToComponents(&transformMatrix[0][0], &transform.Translation[0], &transform.Rotation[0], &transform.Scale[0]);
						}
					}
				}
				else
				{
					mWidgetRotation = false;
					mWidgetScale = false;
					mWidgetTranslation = false;
				}
			}
		}
	}
	ImGui::PopStyleVar();
	ImGui::End();
}
