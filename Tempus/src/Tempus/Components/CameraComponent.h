// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "Component.h"

namespace Tempus 
{
	enum class CamProjectionType : uint8_t
	{
		Perspective,
		Orthographic
	};

	class TEMPUS_API CameraComponent : public Component
	{
		DECLARE_COMPONENT(CameraComponent, 1)
		TPS_DEBUG_NAME("Camera Component")

	public:

		CamProjectionType ProjectionType = CamProjectionType::Perspective;

		float Fov = 60.0f;
		float OrthoSize = 10.0f;
		float NearClip = 0.1f;
		float FarClip = 1000.0f;
		float AspectRatio = 16.0f / 9.0f;
	};
}


