#include "TextureViewerPanel.h"

#include <IconsFontAwesome6.h>

void TextureViewerPanel::OnAttach()
{
	mTexture = nullptr;
}

void TextureViewerPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		if (!mTexture)
		{
			ImGui::Text("No Texture");
		}
		else
		{
			const float offset = 100.f;
			float width = ImGui::GetContentRegionAvail().x;
			width = glm::min(width, 400.f);

			ImTextureID id = 0;
			std::string name = mTexture->Name();
			std::string filepath = mTexture->FilePath().string();
			std::string guid = std::to_string(mTexture->Handle());
			std::string format = Mule::GetTextureFormatName(mTexture->GetFormat());

			ImGui::BeginDisabled();

			ImGui::Text("Name"); ImGui::SameLine(offset);
			ImGui::PushItemWidth(width - offset);
			ImGui::InputText("##Name", name.data(), name.size());

			ImGui::Text("Filepath"); ImGui::SameLine(offset);
			ImGui::PushItemWidth(width - offset);
			ImGui::InputText("##Filepath", filepath.data(), filepath.size());

			ImGui::Text("Guid"); ImGui::SameLine(offset);
			ImGui::PushItemWidth(width - offset);
			ImGui::InputText("##Guid", guid.data(), guid.size());

			ImGui::Text("Format"); ImGui::SameLine(offset);
			ImGui::PushItemWidth(width - offset);
			ImGui::InputText("##Format", format.data(), format.size());

			ImGui::EndDisabled();

			
			if(mTexture->GetTextureType() == Mule::TextureType::Type_2D)
			{
				WeakRef<Mule::Texture2D> tex2d = mTexture;
				id = tex2d->GetImGuiID();
			}
			
			if (mTexture->GetLayerCount() > 1)
			{
				ImGui::Text("Layer"); ImGui::SameLine(offset);
				ImGui::PushItemWidth(width - offset);
				ImGui::DragInt("##Layer", &mLayer, 0.02f, 0, mTexture->GetLayerCount() - 1);
			}
			if (mTexture->GetMipCount() > 1)
			{
				ImGui::Text("Mip Level"); ImGui::SameLine(offset);
				ImGui::PushItemWidth(width - offset);
				ImGui::DragInt("##Mip", &mMipLevel, 0.02f, 0, mTexture->GetMipCount() - 1);
			}

			id = mTexture->GetImGuiMipLayerID(mMipLevel, mLayer);

			if (id != 0)
			{
				ImGui::Image(id, { 512, 512 });
			}
		}
	}
	ImGui::End();
}

void TextureViewerPanel::OnEvent(Ref<IEditorEvent> event)
{
}

void TextureViewerPanel::SetTexture(Mule::AssetHandle textureHandle)
{
	auto texture = mEngineContext->GetAsset<Mule::ITexture>(textureHandle);
	mTexture = texture;
	mMipLevel = 0;
	mLayer = 0;
}
