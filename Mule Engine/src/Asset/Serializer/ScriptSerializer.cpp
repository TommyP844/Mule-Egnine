#include "Asset/Serializer/ScriptSerializer.h"

namespace Mule
{
    ScriptSerializer::ScriptSerializer(Ref<ServiceManager> serviceManager)
        :
        IAssetSerializer(serviceManager)
    {
    }

    Ref<ScriptClass> ScriptSerializer::Load(const fs::path& filepath)
    {
        return MakeRef<ScriptClass>(filepath);
    }

    void ScriptSerializer::Save(Ref<ScriptClass> asset)
    {
    }
}
