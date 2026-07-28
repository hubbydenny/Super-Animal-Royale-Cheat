#include "EspModule.hpp"
#include "../../Core/Hooks/HooksManager.hpp"
#include "../../Core/Hooks/HooksDefinitions.hpp"
#include <spdlog/spdlog.h>
#include <cmath>
#include "../Source/Core/GameStructs/GameStructs.hpp"
#include <imgui.h>
#include <Windows.h>
#include <unordered_set>

static bool ReadIl2CppString(void* strPtr, char* outBuf, size_t maxLen)
{
	if (!strPtr || IsBadReadPtr(strPtr, 0x18)) return false;
	int32_t len = *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(strPtr) + 0x10);
	if (len <= 0 || len > 256) return false;
	const wchar_t* wChars = reinterpret_cast<const wchar_t*>(reinterpret_cast<const char*>(strPtr) + 0x14);
	if (IsBadReadPtr(wChars, len * sizeof(wchar_t))) return false;

	int count = 0;
	for (; count < static_cast<int>(maxLen) - 1 && count < len; count++)
	{
		wchar_t wc = wChars[count];
		if (wc == 0) break;
		outBuf[count] = (wc < 128) ? static_cast<char>(wc) : '?';
	}
	outBuf[count] = '\0';
	return count > 0;
}

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
	if (!cfg.kbEsp.UpdateState() || !cfg.kbNames.UpdateState()) return;
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
	if (!localNet || !localNet->gameCamera || snapshot.empty()) return;
	float orthoSize = localNet->gameCamera->mainOrthoSize;
	if (orthoSize <= 0.001f) return;

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		float pixelsPerUnit = ScreenY / (orthoSize * 2.0f);
		float cx = ScreenX * 0.5f;
		float cy = ScreenY * 0.5f;
		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float screenX = cx + dx * pixelsPerUnit;
		float screenY = cy - dy * pixelsPerUnit;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist < 0.1f) continue;

		float scale = 67.5f / orthoSize;
		float rbh = roundf(60.0f * scale);
		float topY = screenY - rbh;

		char narrowBuf[128] = { 0 };
		if (!ReadIl2CppString(target->playerName, narrowBuf, sizeof(narrowBuf)))
		{
			snprintf(narrowBuf, sizeof(narrowBuf), "Player #%d", target->playerID);
		}

		ImVec2 textSize = ImGui::CalcTextSize(narrowBuf);
		ImU32 colName = ImColor(cfg.colName[0], cfg.colName[1], cfg.colName[2], cfg.colName[3]);
		dl->AddText(ImVec2(screenX - textSize.x * 0.5f, topY - textSize.y - 4.0f),
			colName, narrowBuf);
	}
}

void EspModule::DrawArrows()
{
	if (!cfg.bEsp || !cfg.bArrows) return;
	if (!cfg.kbEsp.UpdateState() || !cfg.kbArrows.UpdateState()) return;
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

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

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
	}
}

void EspModule::DrawSnaplines()
{
	if (!cfg.bEsp || !cfg.bSnaplines) return;
	if (!cfg.kbEsp.UpdateState() || !cfg.kbSnaplines.UpdateState()) return;
	const auto& ctx = hooks->GetGameContext();

	float screenW = ImGui::GetIO().DisplaySize.x;
	float screenH = ImGui::GetIO().DisplaySize.y;
	float ox = screenW * 0.5f;
	float oy = screenH * 0.5f;

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
	if (!localNet || !localNet->gameCamera || snapshot.empty()) return;
	float orthoSize = localNet->gameCamera->mainOrthoSize;
	if (orthoSize <= 0.001f) return;

	int count = 0;

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		float pixelsPerUnit = screenH / (orthoSize * 2.0f);
		float dx = target->previousPosition.x - localPos.x;
		float dy = target->previousPosition.y - localPos.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist > cfg.fMaxSnaplineRenderDistance || dist < 0.1f) continue;

		float targetX = ox + dx * pixelsPerUnit;
		float targetY = oy - dy * pixelsPerUnit;

		count++;

		ImU32 color = dist < cfg.fNearSnaplineRenderDistance
			? IM_COL32(0, 200, 0, 75)
			: IM_COL32(200, 0, 0, 75);

		dl->AddLine(ImVec2(ox, oy), ImVec2(targetX, targetY), color, 1.0f);
	}
}

void EspModule::DrawBoxes()
{
	if (!cfg.bEsp || !cfg.bBoxes) return;
	if (!cfg.kbEsp.UpdateState() || !cfg.kbBoxes.UpdateState()) return;
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
	if (!localNet || !localNet->gameCamera || snapshot.empty()) return;
	float orthoSize = localNet->gameCamera->mainOrthoSize;
	if (orthoSize <= 0.001f) return;

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		float pixelsPerUnit = screenHeight / (orthoSize * 2.0f);
		float cx = screenWidth * 0.5f;
		float cy = screenHeight * 0.5f;
		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float screenX = cx + dx * pixelsPerUnit;
		float screenY = cy - dy * pixelsPerUnit;

		float scale = 67.5f / orthoSize;
		float rbw = roundf(40.0f * scale);
		float rbh = roundf(60.0f * scale);

		float topY = screenY - rbh;
		float bottomY = screenY;

		if (screenX + rbw * 0.5f < 0 || screenX - rbw * 0.5f > screenWidth) continue;
		if (bottomY < 0 || topY > screenHeight) continue;

		ImVec2 bMin(roundf(screenX - rbw * 0.5f), roundf(topY));
		ImVec2 bMax(roundf(screenX + rbw * 0.5f), roundf(bottomY));
		ImU32 colBox = ImColor(cfg.colBox[0], cfg.colBox[1], cfg.colBox[2], cfg.colBox[3]);
		dl->AddRect(bMin, bMax, colBox);
	}
}

void EspModule::DrawArmor()
{
	if (!cfg.bEsp || !cfg.bArmorEsp) return;
	if (!cfg.kbEsp.UpdateState() || !cfg.kbArmorEsp.UpdateState()) return;
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
	if (!localNet || !localNet->gameCamera || snapshot.empty()) return;
	float orthoSize = localNet->gameCamera->mainOrthoSize;
	if (orthoSize <= 0.001f) return;

	std::unordered_set<int16_t> drawnIDsArmor;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDsArmor.count(target->playerID)) continue;
		drawnIDsArmor.insert(target->playerID);

		float pixelsPerUnit = ScreenY / (orthoSize * 2.0f);
		float cx = ScreenX * 0.5f;
		float cy = ScreenY * 0.5f;
		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float screenX = cx + dx * pixelsPerUnit;
		float screenY = cy - dy * pixelsPerUnit;

		if (screenX < 0 || screenX > ScreenX || screenY < 0 || screenY > ScreenY) continue;

		int armorLvl = (int)target->currArmorLvl;
		int armorAmt = target->currArmorAmount;

		if (armorLvl <= 0 && armorAmt <= 0) continue;

		char armorBuf[64];
		snprintf(armorBuf, sizeof(armorBuf), "Armor %d (%d)", armorLvl, armorAmt);

		ImVec2 textSize = ImGui::CalcTextSize(armorBuf);
		ImU32 colArmor = ImColor(cfg.colArmor[0], cfg.colArmor[1], cfg.colArmor[2], cfg.colArmor[3]);

		dl->AddText(ImVec2(screenX - textSize.x * 0.5f, screenY + 4.0f), colArmor, armorBuf);
	}
}

void EspModule::DrawGrenades()
{
	if (!cfg.bEsp || !cfg.bGrenadeEsp) return;
	if (!cfg.kbEsp.UpdateState() || !cfg.kbGrenadeEsp.UpdateState()) return;
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
	if (!localNet || !localNet->gameCamera || snapshot.empty()) return;
	float orthoSize = localNet->gameCamera->mainOrthoSize;
	if (orthoSize <= 0.001f) return;

	std::unordered_set<int16_t> drawnIDsNades;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (target->grenadeCount <= 0) continue;
		if (drawnIDsNades.count(target->playerID)) continue;
		drawnIDsNades.insert(target->playerID);

		float pixelsPerUnit = ScreenY / (orthoSize * 2.0f);
		float cx = ScreenX * 0.5f;
		float cy = ScreenY * 0.5f;
		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float screenX = cx + dx * pixelsPerUnit;
		float screenY = cy - dy * pixelsPerUnit;

		if (screenX < 0 || screenX > ScreenX || screenY < 0 || screenY > ScreenY) continue;

		char nadeBuf[32];
		snprintf(nadeBuf, sizeof(nadeBuf), "Nades: %d", target->grenadeCount);

		ImVec2 textSize = ImGui::CalcTextSize(nadeBuf);
		ImU32 colNade = ImColor(cfg.colGrenade[0], cfg.colGrenade[1], cfg.colGrenade[2], cfg.colGrenade[3]);

		float offsetY = cfg.bArmorEsp ? 20.0f : 4.0f;
		dl->AddText(ImVec2(screenX - textSize.x * 0.5f, screenY + offsetY), colNade, nadeBuf);
	}
}