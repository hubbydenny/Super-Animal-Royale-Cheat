#include "Aimbot.hpp"
#include "../../Core/Hooks/HooksManager.hpp"
#include "../../Core/Hooks/HooksDefinitions.hpp"
#include "../../Core/GameStructs/GameStructs.hpp"
#include <cmath>
#include <Windows.h>

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

	bool keyPressed = (GetAsyncKeyState(cfg.iAimKey) & 0x8000) != 0;
	if (!keyPressed && cfg.iAimMode == 0)
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	const auto& ctx = hooks->GetGameContext();

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

	float bestDist = 999999.0f;
	NetworkPlayer* bestTarget = nullptr;

	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;

		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float dist = std::sqrtf(dx * dx + dy * dy);

		if (dist < bestDist)
		{
			bestDist = dist;
			bestTarget = target;
		}
	}

	if (!bestTarget)
	{
		HooksDefinitions::g_hasAimTarget = false;
		return;
	}

	float dx = bestTarget->previousPosition.x - localNet->previousPosition.x;
	float dy = bestTarget->previousPosition.y - localNet->previousPosition.y;
	float angle = std::atan2f(dy, dx);

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

	float aimDist = 200.0f;
	HooksDefinitions::g_aimTarget.x = (LONG)(cx + std::cosf(angle) * aimDist);
	HooksDefinitions::g_aimTarget.y = (LONG)(cy - std::sinf(angle) * aimDist);
	HooksDefinitions::g_hasAimTarget = true;
}
