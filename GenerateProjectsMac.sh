#!/bin/bash

if [ -z "$VULKAN_SDK" ]; then
    echo "Error: VULKAN_SDK environment variable is not set."
    echo "Please ensure Vulkan SDK is installed and the environment variable is configured."
    read -p "Press any key to exit..." -n1
    exit 1
fi

vendor/bin/premake/premake5 clean
vendor/bin/premake/premake5 gmake2
./CompileShadersMac.sh

read -p "Press any key to continue..."