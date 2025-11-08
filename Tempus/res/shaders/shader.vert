// Copyright Levi Spevakow (C) 2025

#version 450

layout(binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
} global;

layout(binding = 1) uniform ObjectUBO {
    mat4 model;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out vec3 fragLightPos;

void main() {
    gl_Position = global.proj * global.view * object.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragPosition = vec3(object.model * vec4(inPosition, 1.0));
    fragLightPos = global.lightPos;
    fragNormal = mat3(object.model) * inNormal;
}