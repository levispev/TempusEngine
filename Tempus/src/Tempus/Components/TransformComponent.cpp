// Copyright Levi Spevakow (C) 2025

#include "TransformComponent.h"
#include <glm/trigonometric.hpp>
#include <glm/detail/func_geometric.inl>

glm::vec3 Tempus::TransformComponent::GetForwardVector() const
{
    // Rotation is Euler angles in degrees: x = pitch, y = yaw, z = roll
    float pitch = glm::radians(Rotation.x);
    float yaw   = glm::radians(Rotation.y);

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

Tempus::TransformComponent::TransformComponent()
{
}

Tempus::TransformComponent::~TransformComponent()
{
    Component::~Component();
}
