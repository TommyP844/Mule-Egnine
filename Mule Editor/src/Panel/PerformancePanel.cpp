#include "PerformancePanel.h"

PerformancePanel::PerformancePanel()
	:
	IPanel("Performance")
{
}

PerformancePanel::~PerformancePanel()
{
}

void PerformancePanel::OnAttach()
{
}

void PerformancePanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		float ms = dt * 1e3f;
		ImGui::Text("Frame Time: %.3fms", ms);
		ImGui::Text("FPS: %.f", 1.f / dt);

		auto sceneRenderer = mEngineContext->GetSceneRenderer();
		auto stats = sceneRenderer->GetRenderStats();

		ImGui::Text("Data Prep Time: %.3fms", stats.CPUPrepareTime * 1e3f);
		ImGui::Text("Execution Time: %.3fms", stats.CPUExecutionTime * 1e3f);

		for (const auto& renderPassStats : stats.RenderPassStats)
		{
			if (ImGui::TreeNode(renderPassStats.Name.c_str()))
			{
				ImGui::Text("CPU Record Time: %.3f", renderPassStats.CPUExecutionTime * 1e3f);
				ImGui::Text("GPU Execution Time: %.3f", renderPassStats.GPUExecutionTime * 1e3f);
				
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

void PerformancePanel::OnEvent(Ref<IEditorEvent> event)
{
}
