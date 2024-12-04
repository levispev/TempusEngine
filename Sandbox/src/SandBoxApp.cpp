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

int main() 
{
	SandBox* sandbox = new SandBox();
	sandbox->Run();
	delete sandbox;

	return 0;
}