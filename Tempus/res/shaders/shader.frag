// Copyright Levi Spevakow (C) 2025

#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = fragTexCoord;
    // Flipping X coordinate to account for left handed system
    uv.x = 1.0 - uv.x;
    outColor = texture(texSampler, uv);
    outColor += vec4(fragColor * 0.5, 1.0);
}