#include "Asset/Serializer/ScriptSerializer.h"

namespace Mule
{
    Ref<ScriptClass> ScriptSerializer::Load(const fs::path& filepath)
    {
        return MakeRef<ScriptClass>(filepath);
    }

    void ScriptSerializer::Save(Ref<ScriptClass> asset)
    {
    }
}
