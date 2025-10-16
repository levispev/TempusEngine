// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Component.h"
#include "Core.h"

namespace Tempus 
{

	enum class CamProjectionType : uint8_t
	{
		Perspective,
		Orthographic
	};

	class TEMPUS_API CameraComponent : public Component
	{
		DECLARE_COMPONENT(1, "Camera Component")

	public:

		CamProjectionType ProjectionType = CamProjectionType::Perspective;

		float Fov = 90.0f;
		float OrthoSize = 10.0f;
		float NearClip = 0.1f;
		float FarClip = 1000.0f;
		float AspectRatio = 16.0f / 9.0f;

	};

}


