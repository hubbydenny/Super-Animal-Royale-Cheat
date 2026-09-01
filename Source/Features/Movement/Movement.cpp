#include "Movement.hpp"
#include "../../Core/Hooks/HooksManager.hpp"
#include "../../Core/Hooks/HooksDefinitions.hpp"
#include <spdlog/spdlog.h>
#include <Windows.h>

MovementModule::MovementModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg)
	: ModuleBase(MOVEMENT_MODULE_NAME, modules, hooks, cfg)
{}

void MovementModule::Run()
{
	const auto& ctx = hooks->GetGameContext();

	NetworkPlayer* localNet = nullptr;
	LocalPlayerScript* localScript = nullptr;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player) return;
		localNet = ctx.localPlayer->player;
		localScript = ctx.localPlayer;
	}
	if (!localNet) return;

	if (cfg.bAntiBanana && cfg.kbAntiBanana.UpdateState())
	{
		localNet->bananaStunStartTime = 0.0f;
		localNet->bananaStunEndTime = 0.0f;
		if (localNet->currentWalkMode == EWalkMode::BananaStun)
			localNet->currentWalkMode = EWalkMode::Normal;
		if (localScript)
		{
			localScript->lastServerPositionForBananaStun = localScript->lastServerPosition;
		}
	}

	if (cfg.bInfiniteRoll && cfg.kbInfiniteRoll.UpdateState())
	{
		localNet->rollEndTime = 0.0f;
		localNet->rollStartTime = 0.0f;
		localNet->didHitBunnyHopRoll = true;
		localNet->bunnyHopCurrentBonus = 1.0f;
		if (localNet->currentWalkMode == EWalkMode::Roll)
		{
			localNet->currentWalkMode = EWalkMode::Normal;
		}
	}

	if (cfg.bBhop && cfg.kbBhop.UpdateState())
	{
		bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
		if (spaceDown)
		{
			localNet->didHitBunnyHopRoll = true;
			localNet->bunnyHopCurrentBonus = 1.0f;
		}
	}

	if (cfg.bFastParachute && cfg.kbFastParachute.UpdateState())
	{
		if (localNet->currParachuteHeight > 0.0f)
		{
			localNet->currParachuteHeight -= cfg.fParachuteDropSpeed;
			if (localNet->currParachuteHeight < 0.0f)
				localNet->currParachuteHeight = 0.0f;
		}
	}

	if (cfg.bDisableMoveAnim && cfg.kbDisableMoveAnim.UpdateState())
	{
		localNet->isPressingMoveKeys = false;
	}

	if (cfg.bVehicleFly && cfg.kbVehicleFly.UpdateState())
	{
		float speed = cfg.fVehicleFlySpeed * 0.016f;
		if (GetAsyncKeyState('W') & 0x8000) localNet->currentPosition.y += speed;
		if (GetAsyncKeyState('S') & 0x8000) localNet->currentPosition.y -= speed;
		if (GetAsyncKeyState('A') & 0x8000) localNet->currentPosition.x -= speed;
		if (GetAsyncKeyState('D') & 0x8000) localNet->currentPosition.x += speed;
	}

	if (cfg.bVehicleBoost)
	{
		if (localNet->currentWalkMode == EWalkMode::Vehicle)
		{
			float boostSpeed = cfg.fVehicleFlySpeed * 0.03f;
			if (GetAsyncKeyState('W') & 0x8000) localNet->currentPosition.y += boostSpeed;
			if (GetAsyncKeyState('S') & 0x8000) localNet->currentPosition.y -= boostSpeed;
			if (GetAsyncKeyState('A') & 0x8000) localNet->currentPosition.x -= boostSpeed;
			if (GetAsyncKeyState('D') & 0x8000) localNet->currentPosition.x += boostSpeed;
		}
	}
}
