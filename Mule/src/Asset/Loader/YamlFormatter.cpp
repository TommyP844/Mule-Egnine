#include "Asset/Loader/YamlFormatter.h"

Ref<Mule::Scene> YAML::convert<Mule::Entity>::gScene = nullptr;
WeakRef<Mule::ScriptContext> YAML::convert<Mule::ScriptComponent>::gScriptContext = nullptr;
Mule::Entity YAML::convert<Mule::ScriptComponent>::gEntity = Mule::Entity();