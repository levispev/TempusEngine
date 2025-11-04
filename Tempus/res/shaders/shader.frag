// Copyright Levi Spevakow (C) 2025

#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = fragTexCoord;
    // Flipping Y coordinate to account for left handed system
    uv.y = 1.0 - uv.y;
    outColor = texture(texSampler, uv);
}