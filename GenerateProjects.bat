@echo off

if not defined VULKAN_SDK (
    echo Error: VULKAN_SDK environment variable is not set.
    echo Please ensure Vulkan SDK is installed and the environment variable is configured.
    PAUSE
    exit /b 1
)

rmdir /s /q bin
rmdir /s /q bin-int
rmdir /s /q .vs

del Tempus.sln
del "Sandbox\Sandbox.vcxproj"
del "Sandbox\Sandbox.vcxproj.user"
del "Tempus\Tempus.vcxproj"
del "Tempus\Tempus.vcxproj.filters"

call vendor\bin\premake\premake5.exe vs2022
call CompileShaders.bat
PAUSE