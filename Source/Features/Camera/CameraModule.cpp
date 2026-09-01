#include "CameraModule.hpp"
#include "../../Core/Hooks/HooksManager.hpp"
#include <spdlog/spdlog.h>

CameraModule::CameraModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg)
	: ModuleBase(CAMERA_MODULE_NAME, modules, hooks, cfg)
{
}

void CameraModule::Run()
{
	const GameContext& ctx = hooks->GetGameContext();
	
	if (ctx.localPlayer && ctx.localPlayer->player)
	{
		auto netPlayer = ctx.localPlayer->player;
		if (!IsBadReadPtr(netPlayer, sizeof(NetworkPlayer)))
		{
			auto gc = netPlayer->gameCamera;
			if (gc && !IsBadReadPtr(gc, sizeof(GameCamera)))
			{
				if (cfg.bZoomOverride)
				{
					if (cfg.fZoomOverrideValue >= 10.0f && cfg.fZoomOverrideValue <= 1000.0f)
					{
						gc->mainOrthoSize = cfg.fZoomOverrideValue;
					}
				}
			}
		}
	}
}

void CameraModule::Shutdown()
{
	if (pCameraOrtho)
	{
		*pCameraOrtho = cfg.fZoomDefaultValue;
	}
}
