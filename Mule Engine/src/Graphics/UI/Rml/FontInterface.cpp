#include "Graphics/UI/Rml/FontInterface.h"

namespace Mule::UI
{
	void RmlFontInterface::Initialize()
	{
	}

	void RmlFontInterface::Shutdown()
	{
	}

	bool RmlFontInterface::LoadFontFace(const Rml::String& file_name, int face_index, bool fallback_face, Rml::Style::FontWeight weight)
	{
		return false;
	}

	bool RmlFontInterface::LoadFontFace(Rml::Span<const Rml::byte> data, int face_index, const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallback_face)
	{
		return false;
	}

	Rml::FontFaceHandle RmlFontInterface::GetFontFaceHandle(const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size)
	{
		return Rml::FontFaceHandle();
	}

	Rml::FontEffectsHandle RmlFontInterface::PrepareFontEffects(Rml::FontFaceHandle handle, const Rml::FontEffectList& font_effects)
	{
		return Rml::FontEffectsHandle();
	}

	const Rml::FontMetrics& RmlFontInterface::GetFontMetrics(Rml::FontFaceHandle handle)
	{
		return Rml::FontMetrics();
	}

	int RmlFontInterface::GetStringWidth(Rml::FontFaceHandle handle, Rml::StringView string, const Rml::TextShapingContext& text_shaping_context, Rml::Character prior_character)
	{
		return 0;
	}

	int RmlFontInterface::GenerateString(Rml::RenderManager& render_manager, Rml::FontFaceHandle face_handle, Rml::FontEffectsHandle font_effects_handle, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied colour, float opacity, const Rml::TextShapingContext& text_shaping_context, Rml::TexturedMeshList& mesh_list)
	{
		return 0;
	}

	int RmlFontInterface::GetVersion(Rml::FontFaceHandle handle)
	{
		return 0;
	}

	void RmlFontInterface::ReleaseFontResources()
	{
	}
}