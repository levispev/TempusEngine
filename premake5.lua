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

-- Project root directory (absolute path)
projectRoot = path.getabsolute(".")

project "Tempus"
    location "Tempus"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/src/**.cpp"
    }

    includedirs
    {
        path.join(os.getenv("VULKAN_SDK"), "Include"),
        "%{prj.name}/vendor/include",
        "%{prj.name}/src/",
        "%{prj.name}/src/Tempus",
        "%{prj.name}/vendor/include/imgui"
    }

    libdirs
    {
        path.join(os.getenv("VULKAN_SDK"), "Lib"),
        "%{prj.name}/vendor/bin/sdl/"
    }

    filter "system:windows"
        cppdialect "C++23"
        staticruntime "Off"
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
            "TPS_BUILD_DLL",
            "TPS_PROJECT_ROOT=\"" .. projectRoot .. "\"",
            "TPS_CONTENT_DIR=\"" .. path.join(projectRoot, "Tempus/res") .. "\"",
            "TPS_SHADER_DIR=\"" .. path.join(projectRoot, "Tempus/res/shaders") .. "\"",
            "TPS_TEXTURE_DIR=\"" .. path.join(projectRoot, "Tempus/res/textures") .. "\""
        }

        buildoptions
        {
            "/utf-8",
            "/wd4251"
        }

        postbuildcommands
        {
            "if not exist \"../bin/" .. outputdir .. "/Sandbox\" mkdir \"../bin/" .. outputdir .. "/Sandbox\"",
            "{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"
        }
    
            -- Override libdirs for Dist to exclude Vulkan SDK. This will link it to vulkan-1.dll in System32 instead.
    filter {"system:windows", "configurations:Dist"}
        libdirs
        {
            "%{prj.name}/vendor/bin/sdl/"
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
            "TPS_BUILD_DLL",
            "TPS_PROJECT_ROOT=\"" .. projectRoot .. "\"",
            "TPS_CONTENT_DIR=\"" .. path.join(projectRoot, "Tempus/res") .. "\"",
            "TPS_SHADER_DIR=\"" .. path.join(projectRoot, "Tempus/res/shaders") .. "\"",
            "TPS_TEXTURE_DIR=\"" .. path.join(projectRoot, "Tempus/res/textures") .. "\""
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
        defines { 
            "TPS_DEBUG",
            "TPS_CONFIG_NAME=\"Debug\""
        }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { 
            "TPS_RELEASE",
            "TPS_CONFIG_NAME=\"Release\""
        }
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines { 
            "TPS_DIST",
            "TPS_CONFIG_NAME=\"Distribution\""
        }
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
        "Tempus/src/Tempus",
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
        cppdialect "C++23"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "TPS_PLATFORM_WINDOWS"
        }

        buildoptions
        {
            "/utf-8",
            "/wd4251"
        }

        postbuildcommands
        {
            "{COPYFILE} %{wks.location}/Tempus/vendor/bin/sdl/SDL2.dll %{cfg.targetdir}"
        }

    filter {"system:windows", "configurations:Dist"}
        postbuildcommands
        {
            "call %{wks.location}/StageBuild.bat " .. outputdir
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
        defines { 
            "TPS_DEBUG",
            "TPS_CONFIG_NAME=\"Debug\""
        }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { 
            "TPS_RELEASE",
            "TPS_CONFIG_NAME=\"Release\""
        }
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines { 
            "TPS_DIST",
            "TPS_CONFIG_NAME=\"Distribution\""
        }
        optimize "On"
        
        
newaction {
    trigger = "clean",
    description = "Remove all generated build files",
    execute = function()
        print("Cleaning build files...")
        
        if os.host() == "windows" then
            print("Cleaning Windows-specific files...")
            os.remove("**.sln")
            os.remove("**.vcxproj")
            os.remove("**.vcxproj.filters")
            os.remove("**.vcxproj.user")
        elseif os.host() == "macosx" then
            print("Cleaning macOS-specific files...")
            os.remove("Makefile")
            os.remove("**.make")
            -- Clean xcode project files
            os.rmdir("**.xcodeproj")
            os.rmdir("**.xcworkspace")
        end
        
        print("Done.")
    end
}
