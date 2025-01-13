if [ -z "$VULKAN_SDK" ]; then
    echo "Error: VULKAN_SDK environment variable is not set."
    exit 1
fi

"$VULKAN_SDK/bin/glslc" ./Tempus/res/shaders/shader.vert -o ./Tempus/res/shaders/vert.spv
"$VULKAN_SDK/bin/glslc" ./Tempus/res/shaders/shader.frag -o ./Tempus/res/shaders/frag.spv

echo "Successfully compiled shaders."