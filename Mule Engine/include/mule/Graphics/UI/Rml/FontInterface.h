#pragma once

#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/StringUtilities.h>

namespace Mule::UI
{
	class RmlFontInterface : public Rml::FontEngineInterface
	{
	public:
		virtual void Initialize();

		/// Called when RmlUi is being shut down.
		virtual void Shutdown();

		virtual bool LoadFontFace(const Rml::String& file_name, int face_index, bool fallback_face, Rml::Style::FontWeight weight);

		virtual bool LoadFontFace(Rml::Span<const Rml::byte> data, int face_index, const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallback_face);

		virtual Rml::FontFaceHandle GetFontFaceHandle(const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size);

		virtual Rml::FontEffectsHandle PrepareFontEffects(Rml::FontFaceHandle handle, const Rml::FontEffectList& font_effects);

		virtual const Rml::FontMetrics& GetFontMetrics(Rml::FontFaceHandle handle);

		virtual int GetStringWidth(Rml::FontFaceHandle handle, Rml::StringView string, const Rml::TextShapingContext& text_shaping_context,
			Rml::Character prior_character = Rml::Character::Null);

		virtual int GenerateString(Rml::RenderManager& render_manager, Rml::FontFaceHandle face_handle, Rml::FontEffectsHandle font_effects_handle, Rml::StringView string,
			Rml::Vector2f position, Rml::ColourbPremultiplied colour, float opacity, const Rml::TextShapingContext& text_shaping_context, Rml::TexturedMeshList& mesh_list);

		virtual int GetVersion(Rml::FontFaceHandle handle);

		virtual void ReleaseFontResources();
	};
}
