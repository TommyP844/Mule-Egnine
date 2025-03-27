
project "Mule"
    
    language "C#"
    dotnetframework "net8.0"
    kind "SharedLib"
    clr "Unsafe"
    
    propertytags {
        { "AppendTargetFrameworkToOutputPath", "false" },
        { "Nullable", "enable" },
    }


    files {
        "Mule/**.cs"
    }