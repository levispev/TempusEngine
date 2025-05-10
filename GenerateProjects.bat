@echo off

if not defined VULKAN_SDK (
    echo Error: VULKAN_SDK environment variable is not set.
    echo Please ensure Vulkan SDK is installed and the environment variable is configured.
    PAUSE
    exit /b 1
)

call vendor\bin\premake\premake5.exe clean
call vendor\bin\premake\premake5.exe vs2022
call CompileShaders.bat