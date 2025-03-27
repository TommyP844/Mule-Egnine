#include "Asset/Loader/ScriptLoader.h"

namespace Mule
{
    Ref<ScriptClass> ScriptLoader::LoadText(const fs::path& filepath)
    {
        return MakeRef<ScriptClass>(filepath);
    }

    void ScriptLoader::SaveText(Ref<ScriptClass> asset)
    {
    }

    Ref<ScriptClass> ScriptLoader::LoadBinary(const Buffer& buffer)
    {
        return Ref<ScriptClass>();
    }

    void ScriptLoader::SaveBinary(Ref<ScriptClass> asset)
    {
    }
}
