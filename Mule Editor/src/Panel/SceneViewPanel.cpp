#include "SceneViewPanel.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "ImGuiExtension.h"

#include <IconsFontAwesome6.h>

SceneViewPanel::SceneViewPanel()
	: 
	IPanel("Scene View"),
	mTranslationSnap({ 0.f }),
	mRotationSnap({ 0.f }),
	mScaleSnap({ 0.f }),
	mGizmoSnap(nullptr),
	mShowSettings(false),
	mGizmoOp((ImGuizmo::OPERATION)0u),
	mGizmoMode(ImGuizmo::MODE::WORLD),
	mCameraMovementSpeed(10.f),
	mIsWindowFocused(false),
	mOpenEntityPopup(false)
{
}

void SceneViewPanel::OnAttach()
{
	auto assetManager = mEngineContext->GetAssetManager();
	mWidth = 0;
	mHeight = 0;
	mBlackImage = assetManager->LoadAsset<Mule::Texture2D>("../Assets/Textures/Black.png");
	mEditorContext->GetEditorCamera().SetNearPlane(1.f);
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

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	if (ImGui::Begin(mName.c_str(), &mIsOpen, flags))
	{
		mIsWindowFocused = ImGui::IsWindowFocused();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
		ImVec2 region = ImGui::GetContentRegionAvail();
		float height = ImGui::GetTextLineHeight() + ImGui::GetStyle().WindowPadding.y * 2.f;
		if (ImGui::BeginChild("PlayBar", {region.x, height}, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			float buttonWidths = ImGui::CalcTextSize(ICON_FA_PLAY).x + ImGui::CalcTextSize(ICON_FA_STOP).x + ImGui::GetStyle().ItemSpacing.x * 2.f;
			float offset = region.x * 0.5f - buttonWidths * 0.5f;
			ImGui::SameLine(offset);

			ImGui::BeginDisabled(mEngineContext->GetScene() == nullptr);
			if (mEditorContext->GetSimulationState() == SimulationState::Editing)
			{
				if (ImGui::Button(ICON_FA_PLAY))
				{
					mEditorContext->SetSimulationState(SimulationState::Simulation);
				}
			}
			else
			{
				if (ImGui::Button(ICON_FA_PAUSE))
				{
					mEditorContext->SetSimulationState(SimulationState::Paused);
				}
			}			
			ImGui::SameLine();
			
			ImGui::BeginDisabled(mEditorContext->GetSimulationState() == SimulationState::Editing);
			if (ImGui::Button(ICON_FA_STOP))
			{
				mEditorContext->SetSimulationState(SimulationState::Editing);
			}
			ImGui::EndDisabled();
			ImGui::EndDisabled();

			ImGui::SameLine(region.x - ImGui::CalcTextSize(ICON_FA_GEAR).x - ImGui::GetStyle().WindowPadding.x * 2.f - ImGui::GetStyle().ItemInnerSpacing.x * 2.f);

			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			if (ImGui::Button(ICON_FA_GEAR))
			{
				mShowSettings = !mShowSettings;
			}
			
			if (mShowSettings)
			{
				const float settingsWindowWidth = 250.f;
				ImGui::SetNextWindowPos({cursorPos.x - settingsWindowWidth + ImGui::CalcTextSize(ICON_FA_GEAR).x, cursorPos.y + ImGui::GetTextLineHeight() * 2});
				ImGui::SetNextWindowSize({ settingsWindowWidth, 350 });
				if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
				{
					const glm::vec3& cameraPosition = mEditorContext->GetEditorCamera().GetPosition();
					float nearPlane = mEditorContext->GetEditorCamera().GetNearPlane();
					float fov = mEditorContext->GetEditorCamera().GetFOVDegrees();
					float farPlane = mEditorContext->GetEditorCamera().GetFarPlane();

					ImGui::SeparatorText("Camera");

					ImGui::Text("Position: %.2f, %.2f, %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
					if (ImGui::SliderFloat("FOV", &fov, 1.f, 179.f)) mEditorContext->GetEditorCamera().SetFOVDegrees(fov);
					if (ImGui::SliderFloat("Near Plane", &nearPlane, 0.1f, farPlane - 1.f)) mEditorContext->GetEditorCamera().SetNearPlane(nearPlane);
					if (ImGui::SliderFloat("Far Plane", &farPlane, nearPlane + 1.f, 10000.f)) mEditorContext->GetEditorCamera().SetFarPlane(farPlane);

					ImGui::SeparatorText("Gizmos");
					ImGui::Text("Translation Snap");
					ImGui::DragFloat3("##TranslationSnap", &mTranslationSnap[0], 1.f, 0.f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);
					ImGui::Separator();
					ImGui::Text("Rotation Snap");
					ImGui::DragFloat("##RotationSnap", &mRotationSnap[0], 1.f, 0.f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);
					ImGui::Separator();
					ImGui::Text("Scale Snap");
					ImGui::DragFloat("##ScaleSnap", &mScaleSnap[0], 1.f, 0.f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);

					auto sceneRenderer = mEngineContext->GetSceneRenderer();
					if (sceneRenderer)
					{
						auto& debugOptions = sceneRenderer->GetDebugOptions();
						ImGui::SeparatorText("Scene Debug");
						ImGui::Checkbox("Show All Physics Objects", &debugOptions.ShowAllPhysicsObjects);
						ImGui::Checkbox("Show All Lights", &debugOptions.ShowAllLights);
						ImGui::Checkbox("Show Selected Entity Colliders", &debugOptions.ShowSelectedEntityColliders);
						ImGui::Checkbox("Show Selected Entity Lights", &debugOptions.ShowSelectedEntityLights);
					}
				}
				ImGui::End();
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		region = ImGui::GetContentRegionAvail();
		ImTextureID texId = mBlackImage->GetImGuiID();
		if (scene)
		{
			WeakRef<Mule::SceneRenderer> sceneRenderer = mEngineContext->GetSceneRenderer();
			
			if (region.x != mWidth || region.y != mHeight)
			{
				mWidth = region.x;
				mHeight = region.y;
				sceneRenderer->Resize(mWidth, mHeight);
				scene->SetViewportDimension(mWidth, mHeight);
				mEditorContext->GetEditorCamera().SetAspectRatio(mWidth / mHeight);
			}

			WeakRef<Mule::FrameBuffer> frameBuffer = sceneRenderer->GetFrameBuffer();
			WeakRef<Mule::Texture2D> texture = frameBuffer->GetColorAttachment(0);
			texId = texture->GetImGuiID();
		}

		ImGui::Image(texId, region);

		HandleDragDrop();
		UpdateCamera(dt);
		UpdateGizmos(cursorPos);
		HandlePicking(cursorPos);
	}
	ImGui::PopStyleVar();
	ImGui::End();
	
	DisplayPopups();
}

void SceneViewPanel::OnEngineEvent(Ref<Mule::Event> event)
{
	if (!mIsWindowFocused) return;

	Mule::Camera& camera = mEditorContext->GetEditorCamera();
	
	if (event->Type == Mule::EventType::KeyboardEvent)
	{
		WeakRef<Mule::KeyboardEvent> keyboardEvent = event;
		Mule::KeyCode key = keyboardEvent->GetKey();
		Mule::KeyCode modifier = keyboardEvent->GetModifier();
		bool pressed = keyboardEvent->IsKeyPressed();

		if (modifier == Mule::KeyCode::None && pressed == true)
		{
			glm::vec3 cameraPosition = camera.GetPosition();
			bool isEditing = mEditorContext->GetSimulationState() == SimulationState::Editing;

			switch (key)
			{
			case Mule::KeyCode::Key_DELETE:
			{
				auto selectedEntity = mEditorContext->GetSelectedEntity();
				if (selectedEntity)
				{
					mEngineContext->GetScene()->DestroyEntity(selectedEntity);
					mEditorContext->SetSelectedEntity(Mule::Entity());
				}
			}
			break;
			case Mule::KeyCode::Key_R:
				if (mGizmoOp == ImGuizmo::OPERATION::ROTATE)
				{
					mGizmoOp = (ImGuizmo::OPERATION)0u;
					mGizmoSnap = nullptr;
				}
				else
				{
					mGizmoOp = ImGuizmo::OPERATION::ROTATE;
					mGizmoSnap = &mRotationSnap[0];
					mGizmoMode = ImGuizmo::MODE::WORLD;
				}
				break;
			case Mule::KeyCode::Key_T:
				if (mGizmoOp == ImGuizmo::OPERATION::TRANSLATE)
				{
					mGizmoOp = (ImGuizmo::OPERATION)0u;
					mGizmoSnap = nullptr;
				}
				else
				{
					mGizmoOp = ImGuizmo::OPERATION::TRANSLATE;
					mGizmoSnap = &mTranslationSnap[0];
					mGizmoMode = ImGuizmo::MODE::WORLD;
				}
				break;
			case Mule::KeyCode::Key_Y:
				if (mGizmoOp == ImGuizmo::OPERATION::SCALE)
				{
					mGizmoOp = (ImGuizmo::OPERATION)0u;
					mGizmoSnap = nullptr;
				}
				else
				{
					mGizmoOp = ImGuizmo::OPERATION::SCALE;
					mGizmoSnap = &mScaleSnap[0];
					mGizmoMode = ImGuizmo::MODE::LOCAL;
				}
				break;
			case Mule::KeyCode::Key_ESCAPE:
				if (mEditorContext->GetSimulationState() == SimulationState::Simulation)
					mEditorContext->SetSimulationState(SimulationState::Editing);
				break;
			}
			camera.SetPosition(cameraPosition);
		}
		else
		{
			if (modifier == Mule::KeyCode::Mod_CONTROL && key == Mule::KeyCode::Key_D && pressed)
			{
				auto selectedEntity = mEditorContext->GetSelectedEntity();
				if (selectedEntity)
				{
					auto newEntity = mEngineContext->GetScene()->CopyEntity(selectedEntity);
					mEditorContext->SetSelectedEntity(newEntity);
				}
			}
		}
	}
}

void SceneViewPanel::UpdateCamera(float dt)
{
	auto simulationState = mEditorContext->GetSimulationState();
	if (ImGui::IsWindowFocused() && simulationState == SimulationState::Editing)
	{
		Mule::Camera& camera = mEditorContext->GetEditorCamera();
		float speed = mCameraMovementSpeed * dt;
		glm::vec3 cameraPosition = camera.GetPosition();

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
		if (ImGui::IsKeyDown(ImGuiKey_Q))
		{
			cameraPosition += camera.GetWorldUp() * speed;
		}
		if (ImGui::IsKeyDown(ImGuiKey_E))
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
	if (mEditorContext->GetSelectedEntity())
	{
		ImGuizmo::SetOrthographic(false);  // Set to true if using an orthographic camera
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(cursorPos.x, cursorPos.y, mWidth, mHeight);

		if (mGizmoOp != 0)
		{
			Mule::Camera& camera = mEditorContext->GetEditorCamera();
			Mule::TransformComponent& transform = mEditorContext->GetSelectedEntity().GetTransformComponent();
			glm::mat4 proj = camera.GetProj();
			glm::mat4 view = camera.GetView();

			glm::mat4 transformMatrix = mEditorContext->GetSelectedEntity().GetTransform();
			if (ImGuizmo::Manipulate(&view[0][0], 
				&proj[0][0], 
				mGizmoOp,
				mGizmoMode, 
				&transformMatrix[0][0],
				nullptr,
				mGizmoSnap))
			{
				ImGuizmo::DecomposeMatrixToComponents(&transformMatrix[0][0], &transform.Translation[0], &transform.Rotation[0], &transform.Scale[0]);
			}
		}
	}
}

void SceneViewPanel::HandleDragDrop()
{
	auto assetManager = mEngineContext->GetAssetManager();
	ImGuiExtension::DragDropFile ddf;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
	{
		switch (ddf.AssetType)
		{
		case Mule::AssetType::Scene:
		{
			auto scene = assetManager->GetAsset<Mule::Scene>(ddf.AssetHandle);
			mEngineContext->SetScene(scene);
			mEditorContext->SetSelectedEntity(Mule::Entity());
		}
			break;
		}
	}

	ImGuiExtension::DragDropAsset dda;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_ASSET, dda))
	{
		switch (dda.AssetType)
		{
		case Mule::AssetType::Mesh:
		{
			auto scene = mEngineContext->GetScene();
			if (scene)
			{
				auto entity = scene->CreateEntity();
				auto& meshComponent = entity.AddComponent<Mule::MeshComponent>();
				meshComponent.MeshHandle = dda.AssetHandle;				
				auto& transform = entity.GetTransformComponent();
				Mule::Camera& camera = mEditorContext->GetEditorCamera();
				transform.Translation = camera.GetPosition() + camera.GetForwardDir() * 20.f;
				mEditorContext->SetSelectedEntity(entity);
			}
		}
			break;
		}
	}
}

void SceneViewPanel::HandlePicking(ImVec2 cursorPos)
{
	if (!ImGui::IsItemHovered())
		return;

	if (ImGuizmo::IsUsingAny())
		return;

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImVec2 mousePos = ImGui::GetMousePos();

		uint32_t x = mousePos.x - cursorPos.x;
		uint32_t y = mousePos.y - cursorPos.y;

		WeakRef<Mule::SceneRenderer> sceneRenderer = mEngineContext->GetSceneRenderer();
		Mule::Guid guid = sceneRenderer->Pick(x, y);
		if(guid != 0ull)
		{
			auto scene = mEngineContext->GetScene();
			if (scene)
			{
				auto selected = mEditorContext->GetSelectedEntity();
				auto entity = scene->GetEntityByGUID(guid);

				if (selected == entity)
					mOpenEntityPopup = true;
				else if(entity)
					mEditorContext->SetSelectedEntity(entity);
			}
		}
	}	
}

void SceneViewPanel::DisplayPopups()
{
	if (mOpenEntityPopup)
	{
		ImGui::OpenPopup("EntityPopup");
		mOpenEntityPopup = false;
	}

	if (ImGui::BeginPopup("EntityPopup"))
	{
		auto entity = mEditorContext->GetSelectedEntity();
		if (!entity)
			ImGui::CloseCurrentPopup();

		ImGui::Text(entity.Name().c_str());
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
		{
			mEngineContext->GetScene()->DestroyEntity(mEditorContext->GetSelectedEntity());
			mEditorContext->SetSelectedEntity(Mule::Entity());
		}
		ImGui::EndPopup();
	}
}
