#pragma once

#ifdef TPS_PLATFORM_WINDOWS

// External function implemented by application
extern Tempus::Application* Tempus::CreateApplication();

// @TODO Use WinMain
int main(int argc, char** argv)
{
	auto app = Tempus::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#elif TPS_PLATFORM_MAC

// External function implemented by application
extern Tempus::Application* Tempus::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Tempus::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif

