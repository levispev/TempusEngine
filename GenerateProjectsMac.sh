#!/bin/bash

if [ -z "$VULKAN_SDK" ]; then
    echo "Error: VULKAN_SDK environment variable is not set."
    echo "Please ensure Vulkan SDK is installed and the environment variable is configured."
    read -p "Press any key to exit..." -n1
    exit 1
fi

rm -rf bin
rm -rf bin-int

rm -rf Tempus.xcworkspace
rm -rf "Sandbox/Sandbox.xcodeproj"
rm -rf "Tempus/Tempus.xcodeproj"

rm -rf Makefile
rm -rf "Sandbox/Makefile"
rm -rf "Tempus/Makefile"

vendor/bin/premake/premake5 gmake2

read -p "Press any key to continue..."