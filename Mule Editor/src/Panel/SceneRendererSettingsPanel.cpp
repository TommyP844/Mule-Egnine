#include "SceneRendererSettingsPanel.h"

SceneRendererSettingsPanel::SceneRendererSettingsPanel()
	:
	IPanel("Render Settings")
{
}

void SceneRendererSettingsPanel::OnAttach()
{
}

void SceneRendererSettingsPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str()))
	{
		SimulationState state = mEditorContext->GetSimulationState();
		Ref<Mule::ResourceRegistry> registry = nullptr;

		switch (state)
		{
		case SimulationState::Editing:
			registry = mEditorContext->GetEditorCamera().GetRegistry();
			break;
		case SimulationState::Simulation:
		case SimulationState::Paused:
			auto sceneCamera = mEngineContext->GetScene()->GetMainCamera();
			if (sceneCamera)
				registry = sceneCamera->GetRegistry();
			break;
		}

		if (registry)
		{
			if (ImGui::BeginTabBar("ResourceTypes"))
			{
				if (ImGui::BeginTabItem("RenderTargets"))
				{
					mType = Mule::ResourceType::RenderTarget;
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Uniform Buffers"))
				{
					mType = Mule::ResourceType::UniformBuffer;
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Shader Resource Groups"))
				{
					mType = Mule::ResourceType::ShaderResourceGroup;
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			DisplayRegistry(registry);
		}
	}
	ImGui::End();
}

void SceneRendererSettingsPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}

void SceneRendererSettingsPanel::DisplayRegistry(Ref<Mule::ResourceRegistry> registry)
{
	for (const Mule::ResourceHandle& handle : registry->GetResourceHandles())
	{
		if (handle.Type != mType)
			continue;

		switch (mType)
		{
		case Mule::ResourceType::DepthAttachment:
		case Mule::ResourceType::RenderTarget:
			DisplayRenderTargets(handle, registry->GetResource<Mule::Texture2D>(handle, 0), registry);
			break;

		case Mule::ResourceType::ShaderResourceGroup:
			DisplayShaderResourceGroups(handle, registry->GetResource<Mule::ShaderResourceGroup>(handle, 0));
			break;

		case Mule::ResourceType::UniformBuffer:
			DisplayUniformBuffers(handle, registry->GetResource<Mule::UniformBuffer>(handle, 0));
			break;
		}
	}
}

void SceneRendererSettingsPanel::DisplayRenderTargets(const Mule::ResourceHandle& handle, Ref<Mule::Texture2D> renderTarget, Ref<Mule::ResourceRegistry> registry)
{
	const std::string& name = handle.Name;
	if (ImGui::TreeNode(name.c_str()))
	{
		std::string name = handle.Name;
		std::string formatName = Mule::GetTextureFormatName(renderTarget->GetFormat());
		ImGui::Text("Size: %i x %i, Format: %s", renderTarget->GetWidth(), renderTarget->GetHeight(), formatName.c_str());
		ImGui::SameLine();
		if (ImGui::Button("View"))
		{
			registry->SetOutputHandle(handle);
		}
		ImGui::TreePop();
	}
}

void SceneRendererSettingsPanel::DisplayUniformBuffers(const Mule::ResourceHandle& handle, Ref<Mule::UniformBuffer> uniformBuffer)
{
	const std::string& name = handle.Name;
	if (ImGui::TreeNode(name.c_str()))
	{
		uint32_t size = uniformBuffer->GetSize();
		ImGui::Text("Size: %i bytes", size);
		ImGui::TreePop();
	}
}

void SceneRendererSettingsPanel::DisplayShaderResourceGroups(const Mule::ResourceHandle& handle, Ref<Mule::ShaderResourceGroup> srg)
{
	const std::string& name = handle.Name;
	if (ImGui::TreeNode(name.c_str()))
	{
		auto blueprint = srg->GetBlueprint();

		ImGui::TreePop();
	}
}
