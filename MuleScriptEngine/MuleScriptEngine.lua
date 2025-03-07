
project "MuleScriptEngine"
    
    language "C#"
    dotnetframework "net8.0"
    kind "SharedLib"
    clr "Unsafe"
    
    propertytags {
        { "AppendTargetFrameworkToOutputPath", "false" },
        { "Nullable", "enable" },
    }


    files {
        "Source/**.cs"
    }