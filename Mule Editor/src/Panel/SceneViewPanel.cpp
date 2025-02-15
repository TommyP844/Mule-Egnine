#include "SceneViewPanel.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "ImGuiExtension.h"

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
	mBlackImage = mEngineContext->LoadAsset<Mule::Texture2D>("../Assets/Textures/Black.png");
	mEditorContext->EditorRenderSettings.EditorCamera.SetNearPlane(0.01);
}

void SceneViewPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	
	uint32_t flags = 0;

	WeakRef<Mule::Scene> scene = mEngineContext->GetScene();
	if (scene && scene->IsModified())
	{
		flags |= ImGuiWindowFlags_UnsavedDocument;
	}

	ImVec2 cursorPos = ImGui::GetCursorScreenPos();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	if (ImGui::Begin(mName.c_str(), &mIsOpen, flags))
	{
		ImVec2 region = ImGui::GetContentRegionAvail();
		ImTextureID texId = mBlackImage->GetImGuiID();
		if (scene)
		{
			Ref<Mule::SceneRenderer> sceneRenderer = mEngineContext->GetSceneRenderer();
			
			if (region.x != mWidth || region.y != mHeight)
			{
				mWidth = region.x;
				mHeight = region.y;
				sceneRenderer->Resize(mWidth, mHeight);
				mEditorContext->EditorRenderSettings.EditorCamera.SetAspectRatio(mWidth / mHeight);
			}

			Ref<Mule::FrameBuffer> frameBuffer = sceneRenderer->GetCurrentFrameBuffer();
			WeakRef<Mule::Texture2D> texture = frameBuffer->GetColorAttachment(0);
			texId = texture->GetImGuiID();

			auto& sceneRendererSettings = sceneRenderer->GetSettings();
			if (sceneRendererSettings.ViewCascadedShadowMaps)
			{
				texId = sceneRenderer->GetShadowPass()->GetFB()[sceneRendererSettings.CascadeIndex]->GetDepthAttachment()->GetImGuiID();
			}
		}

		ImGui::Image(texId, region);

		HandleDragDrop();
		UpdateCamera(dt);
		UpdateGizmos(cursorPos);
	}
	ImGui::PopStyleVar();
	ImGui::End();
}

void SceneViewPanel::UpdateCamera(float dt)
{
	if (ImGui::IsWindowFocused())
	{
		Mule::Camera& camera = mEditorContext->EditorRenderSettings.EditorCamera;
		glm::vec3 cameraPosition = camera.GetPosition();
		const float speed = 10.f * dt;
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

		static glm::vec2 mousePos = { 0, 0 };
		static bool leftPressed = false;
		static bool prevLeftPressed = false;
		prevLeftPressed = leftPressed;
		leftPressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);

		if (!ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				glm::vec2 curPos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
				glm::vec2 delta = curPos - mousePos;
				if (leftPressed && !prevLeftPressed)
					delta = { 0, 0 };
				mousePos = curPos;
				camera.Rotate(delta.x * -0.1f, delta.y * 0.1f);
			}
		}
	}
}

void SceneViewPanel::UpdateGizmos(ImVec2 cursorPos)
{
	if (ImGui::IsWindowFocused())
	{
		if (ImGui::IsKeyPressed(ImGuiKey_T))
			mWidgetTranslation = !mWidgetTranslation;
		if (ImGui::IsKeyPressed(ImGuiKey_Y))
			mWidgetRotation = !mWidgetRotation;
		if (ImGui::IsKeyPressed(ImGuiKey_U))
			mWidgetScale = !mWidgetScale;
	}

	if (mEditorContext->SelectedEntity)
	{
		ImGuizmo::SetOrthographic(false);  // Set to true if using an orthographic camera
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(cursorPos.x, cursorPos.y, mWidth, mHeight);

		uint32_t operation = 0;
		if (mWidgetTranslation) operation |= ImGuizmo::OPERATION::TRANSLATE;
		if (mWidgetRotation) operation |= ImGuizmo::OPERATION::ROTATE;
		if (mWidgetScale) operation |= ImGuizmo::OPERATION::SCALE;

		if (operation != 0)
		{
			Mule::Camera& camera = mEditorContext->EditorRenderSettings.EditorCamera;
			Mule::TransformComponent& transform = mEditorContext->SelectedEntity.GetTransformComponent();
			glm::mat4 proj = camera.GetProj();
			glm::mat4 view = camera.GetView();

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

void SceneViewPanel::HandleDragDrop()
{
	ImGuiExtension::DragDropFile ddf;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
	{
		switch (ddf.AssetType)
		{
		case Mule::AssetType::Scene:
		{
			auto scene = mEngineContext->GetAsset<Mule::Scene>(ddf.AssetHandle);
			mEngineContext->SetScene(scene);
		}
			break;
		}
	}
}
