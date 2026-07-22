#include "EspModule.hpp"
#include "../../Core/Hooks/HooksManager.hpp"
#include "../../Core/Hooks/HooksDefinitions.hpp"
#include <spdlog/spdlog.h>
#include <cmath>
#include "../Source/Core/GameStructs/GameStructs.hpp"
#include <imgui.h>

EspModule::EspModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg)
	: ModuleBase(ESP_MODULE_NAME, modules, hooks, cfg)
{
}

void EspModule::Run()
{
}

void EspModule::DrawBoxes()
{
}

void EspModule::DrawNames()
{
	if (!cfg.bEsp || !cfg.bNames) return;
	const auto& ctx = hooks->GetGameContext();

	float cx = ImGui::GetIO().DisplaySize.x * 0.5f;
	float cy = ImGui::GetIO().DisplaySize.y * 0.5f;
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl) return;

	const float nameRadius = 130.0f;

	NetworkPlayer* localNet = nullptr;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player) return;
		localNet = ctx.localPlayer->player;
	}
	if (!localNet) return;

	std::vector<NetworkPlayer*> snapshot;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		snapshot.assign(ctx.players.begin(), ctx.players.end());
	}

	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;

		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist < 0.1f) continue;

		wchar_t* wName = (wchar_t*)((char*)target->playerName + 0x14);
		if (!wName) continue;

		char narrow[128];
		int i = 0;
		for (; i < 127 && wName[i]; i++)
			narrow[i] = (char)wName[i];
		narrow[i] = '\0';

		float angle = std::atan2f(dy, dx);
		float nx = cx + std::cosf(angle) * nameRadius;
		float ny = cy - std::sinf(angle) * nameRadius;

		ImVec2 textSize = ImGui::CalcTextSize(narrow);
		dl->AddText(ImVec2(nx - textSize.x * 0.5f, ny - 8.0f),
			IM_COL32(255, 255, 255, 255), narrow);

		char hpBuf[32];
		snprintf(hpBuf, sizeof(hpBuf), "%.0f", target->playerHP);
		dl->AddText(ImVec2(nx - 8.0f, ny + 8.0f),
			IM_COL32(0, 255, 0, 200), hpBuf);
	}
}

void EspModule::DrawArrows()
{
	if (!cfg.bEsp || !cfg.bArrows) return;
	const auto& ctx = hooks->GetGameContext();

	float cx = ImGui::GetIO().DisplaySize.x * 0.5f;
	float cy = ImGui::GetIO().DisplaySize.y * 0.5f;
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl) return;

	const float R = 80.0f;
	const float HL = 10.0f;

	NetworkPlayer* localNet = nullptr;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player) return;
		localNet = ctx.localPlayer->player;
	}
	if (!localNet) return;

	std::vector<NetworkPlayer*> snapshot;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		snapshot.assign(ctx.players.begin(), ctx.players.end());
	}

	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;

		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist < 0.1f) continue;

		float angle = std::atan2f(dy, dx);
		float tipX = cx + std::cosf(angle) * R;
		float tipY = cy - std::sinf(angle) * R;
		float pa = angle + 3.14159265f;
		float ha = 0.4f;
		float tailX = cx + std::cosf(angle) * (R - 20.0f);
		float tailY = cy - std::sinf(angle) * (R - 20.0f);

		ImU32 color;
		if (dist < cfg.fNearSnaplineRenderDistance)
			color = IM_COL32(0, 255, 0, 255);
		else if (dist < cfg.fMaxSnaplineRenderDistance)
			color = IM_COL32(255, 0, 0, 255);
		else
			continue;

		dl->AddLine(ImVec2(tipX, tipY), ImVec2(tipX + std::cosf(pa - ha) * HL, tipY - std::sinf(pa - ha) * HL), color, 2.0f);
		dl->AddLine(ImVec2(tipX, tipY), ImVec2(tipX + std::cosf(pa + ha) * HL, tipY - std::sinf(pa + ha) * HL), color, 2.0f);
	}

	if (cfg.bCircle)
		dl->AddCircle(ImVec2(cx, cy), R, IM_COL32(255, 255, 255, 80), 64, 1.0f);
}

void EspModule::DrawSnaplines()
{
	if (!cfg.bEsp || !cfg.bSnaplines) return;
	const auto& ctx = hooks->GetGameContext();

	float screenW = ImGui::GetIO().DisplaySize.x;
	float screenH = ImGui::GetIO().DisplaySize.y;
	float ox = screenW * 0.5f;
	float oy = 0.0f;

	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl) return;

	NetworkPlayer* localNet = nullptr;
	Vector2 localPos(0, 0);
	std::vector<NetworkPlayer*> snapshot;

	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player) return;
		localNet = ctx.localPlayer->player;
		localPos = localNet->currentPosition;
		snapshot.assign(ctx.players.begin(), ctx.players.end());
	}
	if (!localNet || snapshot.empty()) return;

	int count = 0;

	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;

		float dx = target->currentPosition.x - localPos.x;
		float dy = target->currentPosition.y - localPos.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist > cfg.fMaxSnaplineRenderDistance || dist < 1.0f) continue;

		count++;
		float angle = std::atan2f(dy, dx);

		ImU32 color = dist < cfg.fNearSnaplineRenderDistance
			? IM_COL32(0, 255, 0, 200)
			: IM_COL32(255, 0, 0, 200);

		float len = 200.0f + dist * 0.5f;
		if (len > 800.0f) len = 800.0f;

		dl->AddLine(ImVec2(ox, oy),
			ImVec2(ox + std::cosf(angle) * len, oy - std::sinf(angle) * len),
			color, 2.0f);
	}

	if (count > 0)
	{
		char buf[64];
		snprintf(buf, sizeof(buf), "Snaplines: %d", count);
		ImVec2 ts = ImGui::CalcTextSize(buf);
		dl->AddText(ImVec2((screenW - ts.x) * 0.5f, screenH - 20.0f), IM_COL32(255, 255, 0, 255), buf);
	}
}
