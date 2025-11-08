// Copyright Levi Spevakow (C) 2025

#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPosition;
layout(location = 3) in vec3 fragLightPos;

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = fragTexCoord;
    // Flipping Y coordinate to account for left handed system
    uv.y = 1.0 - uv.y;
    
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(fragLightPos - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec4 texColor = texture(texSampler, uv);
    outColor = vec4(texColor.rgb * diff, texColor.a);
}