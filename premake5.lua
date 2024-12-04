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
    }

    libdirs
    {
        path.join(os.getenv("VULKAN_SDK"), "Lib"),
        "%{prj.name}/vendor/bin/glfw/lib-vc2022/",
        "%{prj.name}/vendor/bin/sdl/"
    }

    links
    {
        "vulkan-1",
        "SDL2",
        "SDL2main"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "TPS_PLATFORM_WINDOWS",
            "TPS_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
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

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "TPS_PLATFORM_WINDOWS",
            "GLFW_INCLUDE_NONE"
        }

        buildoptions
        {
            "/utf-8"
        }

        postbuildcommands
        {
            "{COPYFILE} %{wks.location}/Tempus/vendor/bin/sdl/SDL2.dll %{cfg.targetdir}"
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