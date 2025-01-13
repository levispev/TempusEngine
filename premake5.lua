workspace "Tempus"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Tempus"
    location "Tempus"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        path.join(os.getenv("VULKAN_SDK"), "Include"),
        "%{prj.name}/vendor/include",
        "%{prj.name}/src/",
        "%{prj.name}/src/Tempus"
    }

    libdirs
    {
        path.join(os.getenv("VULKAN_SDK"), "Lib"),
        "%{prj.name}/vendor/bin/sdl/"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        links
        {
            "vulkan-1",
            "SDL2",
            "SDL2main"
        }

        defines
        {
            "TPS_PLATFORM_WINDOWS",
            "TPS_BUILD_DLL"
        }

        buildoptions
        {
            "/utf-8"
        }

        postbuildcommands
        {
            "{RMDIR} ../bin/" .. outputdir .. "/Sandbox",
            "{MKDIR} ../bin/" .. outputdir .. "/Sandbox",
            "{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox",
        }

    filter "system:macosx"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "14"
        toolset "clang"

        links
        {
            "MoltenVK",
            "SDL2.framework"
        }

        linkoptions 
        {
            "-rpath /Library/Frameworks",
            "-rpath " .. path.join(os.getenv("VULKAN_SDK"), "Lib")
        }

        frameworkdirs
        {
            "/Library/Frameworks"
        }

        defines
        {
            "TPS_PLATFORM_MAC",
            "TPS_BUILD_DLL"
        }

        postbuildcommands
        {
            "{RMDIR} ../bin/" .. outputdir .. "/Sandbox",
            "{MKDIR} ../bin/" .. outputdir .. "/Sandbox"
        }

        externalincludedirs
        {
            "%{prj.name}/vendor/include"
        }

    filter "configurations:Debug"
        defines "TPS_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "TPS_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "TPS_DIST"
        optimize "On"



project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Tempus/src",
        path.join(os.getenv("VULKAN_SDK"), "Include"),
        "Tempus/vendor/include"
    }

    links
    {
        "Tempus:shared"
    }

    dependson
    {
        "Tempus"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "TPS_PLATFORM_WINDOWS"
        }

        buildoptions
        {
            "/utf-8"
        }

        postbuildcommands
        {
            "{COPYFILE} %{wks.location}/Tempus/vendor/bin/sdl/SDL2.dll %{cfg.targetdir}"
        }

    
    filter "system:macosx"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "14"
        toolset "clang"

        defines
        {
            "TPS_PLATFORM_MAC"
        }

    filter "configurations:Debug"
        defines "TPS_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "TPS_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "TPS_DIST"
        optimize "On"