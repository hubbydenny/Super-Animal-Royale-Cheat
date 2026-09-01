#include "Aimbot.hpp"
#include "../../Core/Hooks/HooksManager.hpp"
#include "../../Core/Hooks/HooksDefinitions.hpp"
#include "../../Core/GameStructs/GameStructs.hpp"
#include <cmath>
#include <cfloat>
#include <Windows.h>
#include <spdlog/spdlog.h>
#include "../Source/Core/Config/Config.hpp"
#include "imgui.h"

AimbotModule::AimbotModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg)
	: ModuleBase(AIMBOT_MODULE_NAME, modules, hooks, cfg)
{
}

void AimbotModule::Run()
{
	if (!cfg.bAimbot)
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	if (!cfg.kbAim.UpdateState())
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	if (cfg.bAimOnlyWhenWeapon && !cfg.is_weapon)
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	const auto& ctx = hooks->GetGameContext();
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl) return;

	NetworkPlayer* localNet = nullptr;
	std::vector<NetworkPlayer*> snapshot;

	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player)
		{
			HooksDefinitions::g_hasAimTarget = false;
			return;
		}
		localNet = ctx.localPlayer->player;
		snapshot.assign(ctx.players.begin(), ctx.players.end());
	}

	if (!localNet || snapshot.empty())
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	static int debugCounter = 0;
	if (++debugCounter % 60 == 0)  // Log every 60 frames (~1 sec at 60fps)
		spdlog::info("[Aimbot] Players: {}, LocalHP: {}", snapshot.size(), localNet->playerHP);

	HWND hwnd = hooks->GetGraphicsContext().hwnd;
	if (!hwnd)
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	RECT rect;
	GetWindowRect(hwnd, &rect);
	float cx = (rect.left + rect.right) * 0.5f;
	float cy = (rect.top + rect.bottom) * 0.5f;

	float maxDistSq   = cfg.fAimFov * cfg.fAimFov;
	NetworkPlayer* bestTarget = nullptr;

	float bestDist    = FLT_MAX;   // Closest
	float bestHP      = FLT_MAX;   // LowestHP  (want minimum)
	float worstHP     = -FLT_MAX;  // HighestHP (want maximum)
	int   bestKills   = -1;        // MostKills (want maximum)

	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;

		if (cfg.bAimIgnoreSniper)
		{
			if (PlayerIsHoldingMelee(target)) continue;
		}

		float dx     = target->previousPosition.x - localNet->previousPosition.x;
		float dy     = target->previousPosition.y - localNet->previousPosition.y;
		float distSq = dx * dx + dy * dy;
		float dist   = std::sqrtf(distSq);

		if (distSq > maxDistSq) continue;

		switch (static_cast<EAimbotMode>(cfg.iAimbotMode))
		{
		case EAimbotMode::Closest:
			if (dist < bestDist)
			{
				bestDist   = dist;
				bestTarget = target;
			}
			break;

		case EAimbotMode::LowestHP:
			if (target->playerHP < bestHP)
			{
				bestHP     = target->playerHP;
				bestDist   = dist;   // keep distance for prediction
				bestTarget = target;
			}
			break;

		case EAimbotMode::HighestHP:
			if (target->playerHP > worstHP)
			{
				worstHP    = target->playerHP;
				bestDist   = dist;
				bestTarget = target;
			}
			break;

		case EAimbotMode::MostKills:
			if (target->numKills > bestKills ||
				(target->numKills == bestKills && dist < bestDist))
			{
				bestKills  = target->numKills;
				bestDist   = dist;
				bestTarget = target;
			}
			break;
		}
	}

	if (!bestTarget)
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	if (cfg.bAimFovCircle)
	{
		ImU32 circleColor = IM_COL32(
			(int)(cfg.colFovCircle[0] * 255.0f),
			(int)(cfg.colFovCircle[1] * 255.0f),
			(int)(cfg.colFovCircle[2] * 255.0f),
			(int)(cfg.colFovCircle[3] * 255.0f)
		);
		dl->AddCircle(ImVec2(cx, cy), cfg.fAimFov, circleColor, 64, 1.5f);
	}

	float targetX = bestTarget->previousPosition.x;
	float targetY = bestTarget->previousPosition.y;

	if (cfg.bAimPrediction)
	{
		float targetVx = bestTarget->currentPosition.x - bestTarget->previousPosition.x;
		float targetVy = bestTarget->currentPosition.y - bestTarget->previousPosition.y;

		float bulletSpeed   = (cfg.fAimBulletSpeed > 1.0f) ? cfg.fAimBulletSpeed : 800.0f;
		float timeOfFlight  = (bestDist / bulletSpeed) * cfg.fAimPredictionFactor;

		targetX += targetVx * 60.0f * timeOfFlight;
		targetY += targetVy * 60.0f * timeOfFlight;
	}

	float dx    = targetX - localNet->previousPosition.x;
	float dy    = targetY - localNet->previousPosition.y;
	float angle = std::atan2f(dy, dx);

	if (cfg.bSilentAim)
	{
		localNet->currentGunAngleToMouse = angle;
		HooksDefinitions::g_hasAimTarget = false;
	}
	else
	{
		float AimDist = 200.0f;
		HooksDefinitions::g_aimTarget.x = (LONG)(cx + std::cosf(angle) * AimDist);
		HooksDefinitions::g_aimTarget.y = (LONG)(cy - std::sinf(angle) * AimDist);
		HooksDefinitions::g_hasAimTarget = true;
	}
}
