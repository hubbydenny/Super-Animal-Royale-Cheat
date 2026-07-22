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

void EspModule::DrawNames()
{
	if (!cfg.bEsp || !cfg.bNames) return;
	const auto& ctx = hooks->GetGameContext();

	float ScreenX = ImGui::GetIO().DisplaySize.x;
	float ScreenY = ImGui::GetIO().DisplaySize.y;

	ImDrawList* dl = ImGui::GetForegroundDrawList();
	if (!dl) return;
	NetworkPlayer* localNet = nullptr;
	std::vector<NetworkPlayer*> snapshot;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player) return;
		localNet = ctx.localPlayer->player;
		snapshot.assign(ctx.players.begin(), ctx.players.end());
	}
	if (!localNet || snapshot.empty()) return;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;

		float orthoSize = ctx.localPlayer->player->gameCamera->mainOrthoSize;
		float pixelsPerUnit = ScreenY / (orthoSize * 2);
		float cx = ScreenX * 0.5f;
		float cy = ScreenY * 0.5f;
		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float screenX = cx + dx * pixelsPerUnit;
		float screenY = cy - dy * pixelsPerUnit;
		float dist = std::sqrtf(dx * dx + dy * dy);
		float TextY = 3.0f * pixelsPerUnit;
		float TextX = 0.0 * pixelsPerUnit;
		if (dist < 0.1f) continue;

		wchar_t* wName = (wchar_t*)((char*)target->playerName + 0x14);
		if (!wName) continue;

		char narrow[128];
		int i = 0;
		for (; i < 127 && wName[i]; i++)
			narrow[i] = (char)wName[i];
		narrow[i] = '\0';

		ImVec2 textSize = ImGui::CalcTextSize(narrow);
		dl->AddText(ImVec2(dx - TextX * 0.5f, TextY - 8.0f),
			IM_COL32(255, 255, 255, 255), narrow);

	//	char hpBuf[32];
	//	snprintf(hpBuf, sizeof(hpBuf), "%.0f", target->playerHP);
	//	dl->AddText(ImVec2(dx - 8.0f, dy + 8.0f),
		//	IM_COL32(0, 255, 0, 200), hpBuf);
	}
}
//void EspModule::DrawHealth() {

//};
void EspModule::DrawArrows()
{
	if (!cfg.bEsp || !cfg.bArrows) return;
	const auto& ctx = hooks->GetGameContext();

	float cx = ImGui::GetIO().DisplaySize.x * 0.5f;
	float cy = ImGui::GetIO().DisplaySize.y * 0.5f;
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl) return;

	const float HL = 10.0f;

	NetworkPlayer* localNet = nullptr;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player) return;
		localNet = ctx.localPlayer->player;
	}
	if (!localNet) return;

	float R = cfg.fArrowsRadius;

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

		ImU32 color;
		if (dist < cfg.fNearSnaplineRenderDistance)
			color = IM_COL32(0, 255, 0, 255);
		else if (dist < cfg.fMaxSnaplineRenderDistance)
			color = IM_COL32(255, 0, 0, 255);
		else
			continue;

		dl->AddLine(ImVec2(tipX, tipY), ImVec2(tipX + std::cosf(pa - ha) * HL, tipY - std::sinf(pa - ha) * HL), color, cfg.fArrowThickness);
		dl->AddLine(ImVec2(tipX, tipY), ImVec2(tipX + std::cosf(pa + ha) * HL, tipY - std::sinf(pa + ha) * HL), color, cfg.fArrowThickness);
		if (cfg.bCircle) return;
			dl->AddCircle(ImVec2(cx, cy), R, IM_COL32(255, 255, 255, 80), 64, 1.0f);
	}
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
		localPos = localNet->previousPosition;
		snapshot.assign(ctx.players.begin(), ctx.players.end());
	}
	if (!localNet || snapshot.empty()) return;

	int count = 0;

	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;

		float dx = target->previousPosition.x - localPos.x;
		float dy = target->previousPosition.y - localPos.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist > cfg.fMaxSnaplineRenderDistance || dist < 1.0f) continue;

		count++;
		float angle = std::atan2f(dy, dx);

		ImU32 color = dist < cfg.fNearSnaplineRenderDistance
			? IM_COL32(0, 255, 0, 200)
			: IM_COL32(255, 0, 0, 200);

		float len = cfg.fSnaplineLength;

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
void EspModule::DrawBoxes()
{
	if (!cfg.bEsp || !cfg.bBoxes) return;
	const auto& ctx = hooks->GetGameContext();

	float screenWidth = ImGui::GetIO().DisplaySize.x;
	float screenHeight = ImGui::GetIO().DisplaySize.y;

	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl) return;
	NetworkPlayer* localNet = nullptr;
	std::vector<NetworkPlayer*> snapshot;

	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.localPlayer || !ctx.localPlayer->player) return;
		localNet = ctx.localPlayer->player;
		snapshot.assign(ctx.players.begin(), ctx.players.end());
	}
	if (!localNet || snapshot.empty()) return;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;

		float orthoSize = ctx.localPlayer->player->gameCamera->mainOrthoSize;
		float pixelsPerUnit = screenHeight / (orthoSize * 2);
		float cx = screenWidth * 0.5f;
		float cy = screenHeight * 0.5f;
		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float screenX = cx + dx * pixelsPerUnit;
		float screenY = cy - dy * pixelsPerUnit;
		float boxW = 5.0f * pixelsPerUnit;
		float boxH = 12.0f * pixelsPerUnit;

		if (screenX + boxW / 2 < 0 || screenX - boxW / 2 > screenWidth) continue;
		if (screenY + boxH / 2 < 0 || screenY - boxH / 2 > screenHeight) continue;

		ImVec2 tl(screenX - boxW / 2, screenY - boxH / 2);
		ImVec2 br(screenX + boxW / 2, screenY + boxH / 2);
		dl->AddRect(tl, br, IM_COL32(255, 0, 0, 255));
	}
}