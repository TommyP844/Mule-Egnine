#pragma once

#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Log.h>

namespace Mule::UI
{
	class RmlSystemInterface : public Rml::SystemInterface
	{
	public:
		virtual double GetElapsedTime();

		virtual int TranslateString(Rml::String& translated, const Rml::String& input);

		virtual void JoinPath(Rml::String& translated_path, const Rml::String& document_path, const Rml::String& path);

		virtual bool LogMessage(Rml::Log::Type type, const Rml::String& message);

		virtual void SetMouseCursor(const Rml::String& cursor_name);

		virtual void SetClipboardText(const Rml::String& text);

		virtual void GetClipboardText(Rml::String& text);

		virtual void ActivateKeyboard(Rml::Vector2f caret_position, float line_height);

		virtual void DeactivateKeyboard();
	};
}