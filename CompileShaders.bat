@echo off
setlocal

if not defined VULKAN_SDK (
    echo Error: VULKAN_SDK environment variable is not set.
    exit /b 1
)

if not exist ".\bin\shaders" (
    mkdir .\bin\shaders
)

"%VULKAN_SDK%\Bin\glslc.exe" .\Tempus\res\shaders\shader.vert -o .\bin\shaders\vert.spv
if %errorlevel% neq 0 (
    echo Error: Failed to compile vertex shader.
    exit /b 1
)

"%VULKAN_SDK%\Bin\glslc.exe" .\Tempus\res\shaders\shader.frag -o .\bin\shaders\frag.spv
if %errorlevel% neq 0 (
    echo Error: Failed to compile fragment shader.
    exit /b 1
)

echo Successfully compiled shaders.