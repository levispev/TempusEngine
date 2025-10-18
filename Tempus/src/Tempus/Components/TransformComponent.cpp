// Copyright Levi Spevakow (C) 2025

#include "TransformComponent.h"
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>


glm::vec3 Tempus::TransformComponent::GetForwardVector() const
{
    float pitch = glm::radians(Rotation.x);
    float yaw = glm::radians(Rotation.y);

    glm::vec3 forward;
    forward.x = glm::cos(pitch) * glm::cos(yaw);
    forward.y = glm::cos(pitch) * glm::sin(yaw);
    forward.z = glm::sin(pitch);

    float len = glm::length(forward);
    if (len <= std::numeric_limits<float>::epsilon())
    {
        return glm::vec3(1.0f, 0.0f, 0.0f);
    }

    return forward / len;
}

glm::vec3 Tempus::TransformComponent::GetRightVector() const
{
    glm::vec3 forward = GetForwardVector();
    glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
    return right;
}

glm::vec3 Tempus::TransformComponent::GetUpVector() const
{
    glm::vec3 forward = GetForwardVector();
    glm::vec3 right = GetRightVector();

    glm::vec3 up = glm::normalize(glm::cross(right, forward));
    return up;
}
