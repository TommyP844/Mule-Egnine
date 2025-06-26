
#include "Graphics/UI/Rml/FileInterface.h"

#include <fstream>
#include <unordered_map>
#include <RmlUi/Core.h>

namespace Mule::UI
{
    // Internal file wrapper to use with Rml::FileHandle
    struct RmlStreamFile {
        std::ifstream stream;
        RmlStreamFile(const Rml::String& path)
        {
            stream = std::ifstream(path, std::ios::binary);
        }

        bool IsOpen() const { return stream.is_open(); }
    };

    Rml::FileHandle RmlFileInterface::Open(const Rml::String& path)
    {
        auto* file = new RmlStreamFile(path);
        if (!file->IsOpen())
        {
            delete file;
            return Rml::FileHandle();
        }

        return reinterpret_cast<Rml::FileHandle>(file);
    }

    void RmlFileInterface::Close(Rml::FileHandle file)
    {
        auto* f = reinterpret_cast<RmlStreamFile*>(file);
        if (f)
        {
            f->stream.close();
            delete f;
        }
    }

    size_t RmlFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
    {
        auto* f = reinterpret_cast<RmlStreamFile*>(file);
        if (!f || !f->IsOpen()) return 0;

        f->stream.read(static_cast<char*>(buffer), size);
        return static_cast<size_t>(f->stream.gcount());
    }

    bool RmlFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
    {
        auto* f = reinterpret_cast<RmlStreamFile*>(file);
        if (!f || !f->IsOpen()) return false;

        std::ios_base::seekdir dir;
        switch (origin)
        {
        case SEEK_SET: dir = std::ios::beg; break;
        case SEEK_CUR: dir = std::ios::cur; break;
        case SEEK_END: dir = std::ios::end; break;
        default: return false;
        }

        f->stream.clear(); // Clear flags in case of previous EOF
        f->stream.seekg(offset, dir);
        return f->stream.good();
    }

    size_t RmlFileInterface::Tell(Rml::FileHandle file)
    {
        auto* f = reinterpret_cast<RmlStreamFile*>(file);
        if (!f || !f->IsOpen()) return 0;

        return static_cast<size_t>(f->stream.tellg());
    }
}
