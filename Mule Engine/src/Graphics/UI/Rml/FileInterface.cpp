#include "Graphics/UI/Rml/FileInterface.h"

namespace Mule::UI
{
    Rml::FileHandle RmlFileInterface::Open(const Rml::String& path)
    {
        return Rml::FileHandle();
    }

    void RmlFileInterface::Close(Rml::FileHandle file)
    {
    }

    size_t RmlFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
    {
        return size_t();
    }

    bool RmlFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
    {
        return false;
    }

    size_t RmlFileInterface::Tell(Rml::FileHandle file)
    {
        return size_t();
    }
}
