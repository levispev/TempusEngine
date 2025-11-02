// Copyright Levi Spevakow (C) 2025

#version 450

layout(binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
} global;

layout(binding = 1) uniform ObjectUBO {
    mat4 model;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = global.proj * global.view * object.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}