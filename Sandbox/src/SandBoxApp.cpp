// Copyright Levi Spevakow (C) 2024

#include "Tempus.h"
#include <iostream>

class SandBox : public Tempus::Application
{
public:

	SandBox() {

	}

	~SandBox() {


	}
};

int main() 
{
	SandBox* sandbox = new SandBox();
	sandbox->Run();
	delete sandbox;
}