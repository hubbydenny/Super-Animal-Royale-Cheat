#include "App.hpp"
#include <Windows.h>
#include <spdlog/spdlog.h>
#include "../Features/ESP/EspModule.hpp"
#include "../Features/Camera/CameraModule.hpp"
#include "../Features/Bunnyhop/Bunnyhop.hpp"
#include "../Features/Aimbot/Aimbot.hpp"
#include "../Features/Misc/Misc.hpp"

App::App()
	: m_logs(),
	m_hooks(),
	m_modules(m_hooks)
{}

void App::Init()
{
	m_logs.Init();
	m_hooks.Init(&m_modules);
	m_modules.Init();
}

void App::AddModules()
{
	m_modules.AddModule<EspModule>();
	m_modules.AddModule<CameraModule>();
	m_modules.AddModule<BunnyhopModule>();
	m_modules.AddModule<AimbotModule>();
	m_modules.AddModule<MiscModule>();
}

void App::Run()
{
	m_hooks.EnableHooks();

	auto& context = m_hooks.GetGraphicsContext();
	while (!GetAsyncKeyState(VK_DELETE) & 1)
	{
		try
		{
			m_modules.Run();
		}
		catch (std::exception& exception)
		{
			spdlog::error(exception.what());
		}
		Sleep(1);
	}

	m_modules.Shutdown();
	m_hooks.DisableHooks();
}

void App::Shutdown()
{
	m_hooks.Shutdown();
	m_logs.Shutdown();
}