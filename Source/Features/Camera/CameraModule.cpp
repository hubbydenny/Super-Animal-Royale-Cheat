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
		if (cfg.bZoomOverride)
		{
			if (!pCameraOrtho)
			{
				auto gc = ctx.localPlayer->player->gameCamera;
			if (gc)
				{
					pCameraOrtho = &gc->mainOrthoSize;
#ifdef _DEBUG
					spdlog::debug("CameraModule: cached ortho ptr {:p}", (void*)pCameraOrtho);
#endif
				}
			}

			if (pCameraOrtho)
			{
				*pCameraOrtho = cfg.fZoomOverrideValue;
			}
		}
		else
		{
			if (pCameraOrtho)
			{
				*pCameraOrtho = cfg.fZoomDefaultValue;
				pCameraOrtho = nullptr;
			}
		}
	}
	else
	{
		if (pCameraOrtho)
		{
#ifdef _DEBUG
			spdlog::debug("CameraModule: localPlayer gone, resetting ortho");
#endif
			*pCameraOrtho = cfg.fZoomDefaultValue;
			pCameraOrtho = nullptr;
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
