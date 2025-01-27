#include "SceneViewPanel.h"

#include "imgui.h"

void SceneViewPanel::OnAttach()
{
	mWidth = 0;
	mHeight = 0;
}

void SceneViewPanel::OnUIRender()
{
	if (!mIsOpen) return;
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		Ref<Mule::Scene> scene = mEngineContext->GetScene();
		if (scene)
		{
			Ref<Mule::SceneRenderer> sceneRenderer = mEngineContext->GetSceneRenderer();

			ImVec2 region = ImGui::GetContentRegionAvail();
			if (region.x != mWidth || region.y != mHeight)
			{
				mWidth = region.x;
				mHeight = region.y;
				sceneRenderer->Resize(mWidth, mHeight);
			}

			Ref<Mule::FrameBuffer> frameBuffer = sceneRenderer->GetCurrentFrameBuffer();
			WeakRef<Mule::Texture2D> texture = frameBuffer->GetColorAttachment(0);
			ImGui::Image(texture->GetImGuiID(), region);
		}
	}
	ImGui::PopStyleVar();
	ImGui::End();
}
