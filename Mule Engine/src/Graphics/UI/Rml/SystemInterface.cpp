#include "Graphics/UI/Rml/SystemInterface.h"

#include <chrono>
#include <sstream>
#include <string>
#include <filesystem>

#include <spdlog/spdlog.h>

namespace Mule::UI
{
    static auto g_start_time = std::chrono::steady_clock::now();

    double RmlSystemInterface::GetElapsedTime()
    {
        using namespace std::chrono;
        auto now = steady_clock::now();
        auto duration = now - g_start_time;
        return duration_cast<std::chrono::seconds>(duration).count();
    }

    int RmlSystemInterface::TranslateString(Rml::String& translated, const Rml::String& input)
    {
        // No translation support, just echo the input
        translated = input;
        return 0;
    }

    void RmlSystemInterface::JoinPath(Rml::String& translated_path, const Rml::String& document_path, const Rml::String& path)
    {
        namespace fs = std::filesystem;

        // If the path is absolute, use it directly
        fs::path p = fs::path(path);
        if (p.is_absolute())
        {
            translated_path = p.generic_string(); // generic_string() uses '/' consistently
            return;
        }

        // Otherwise join with the base path of the document
        fs::path base = fs::path(document_path).parent_path(); // excludes the filename
        fs::path combined = base / p;
        translated_path = combined.generic_string();
    }

    bool RmlSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
    {
        switch (type)
        {
        case Rml::Log::Type::LT_ERROR: SPDLOG_ERROR(message); break;
        case Rml::Log::Type::LT_WARNING: SPDLOG_WARN(message); break;
        case Rml::Log::Type::LT_ASSERT: assert(false && message.c_str()); break;
        case Rml::Log::Type::LT_INFO: SPDLOG_INFO(message); break;
        }

        return true;
    }

    void RmlSystemInterface::SetMouseCursor(const Rml::String& cursor_name)
    {
        // No-op: not supported without OS integration
    }

    void RmlSystemInterface::SetClipboardText(const Rml::String& text)
    {
        // Clipboard not supported in stdlib — no-op
    }

    void RmlSystemInterface::GetClipboardText(Rml::String& text)
    {
        // Clipboard not supported — return empty
        text.clear();
    }

    void RmlSystemInterface::ActivateKeyboard(Rml::Vector2f caret_position, float line_height)
    {
        // No-op: virtual keyboard not applicable in stdlib
    }

    void RmlSystemInterface::DeactivateKeyboard()
    {
    }
}

