#!/bin/bash

if [ -z "$VULKAN_SDK" ]; then
    echo "Error: VULKAN_SDK environment variable is not set."
    exit 1
fi

mkdir -p ./bin/shaders

"$VULKAN_SDK/bin/glslc" ./Tempus/res/shaders/shader.vert -o ./bin/shaders/vert.spv
if [ $? -ne 0 ]; then
    exit 1
fi

"$VULKAN_SDK/bin/glslc" ./Tempus/res/shaders/shader.frag -o ./bin/shaders/frag.spv
if [ $? -ne 0 ]; then
    exit 1
fi

echo "Successfully compiled shaders."