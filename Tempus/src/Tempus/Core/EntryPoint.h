// Copyright Levi Spevakow (C) 2025

#pragma once

#include <exception>
#include <iostream>

#ifdef TPS_PLATFORM_WINDOWS

// External function implemented by application
extern Tempus::Application* Tempus::CreateApplication();

// @TODO Use WinMain
int main(int argc, char** argv)
{
	auto app = Tempus::CreateApplication();

	try
	{
		app->Run();
	}
	catch(const std::exception&)
	{
		return -1;
	}

	delete app;

	return 0;
}

#elif TPS_PLATFORM_MAC

// External function implemented by application
extern Tempus::Application* Tempus::CreateApplication();

int main(int argc, char** argv)
{
#ifdef TPS_DIST
	// Nulling stdout and stderr on distribution
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
#endif

	auto app = Tempus::CreateApplication();

	try
	{
		app->Run();
	}
	catch(const std::exception&)
	{
		return -1;
	}

	delete app;

	return 0;
}

#endif

