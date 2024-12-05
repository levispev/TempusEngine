// Copyright Levi Spevakow (C) 2024

#include "Tempus.h"

class SandBox : public Tempus::Application
{
public:

	SandBox() {

	}

	~SandBox() {


	}
};

Tempus::Application* Tempus::CreateApplication()
{
	return new SandBox();
}