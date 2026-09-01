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
	const wchar_t* wChars = reinterpret_cast<const wchar_t*>(
		reinterpret_cast<const char*>(strPtr) + 0x14);
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

static void WorldToScreen(
	float dx, float dy,
	float screenW, float screenH, float orthoSize,
	float& outX, float& outY)
{
	float ppu = screenH / (orthoSize * 2.0f);
	outX = screenW * 0.5f + dx * ppu;
	outY = screenH * 0.5f - dy * ppu;
}

EspModule::EspModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg)
	: ModuleBase(ESP_MODULE_NAME, modules, hooks, cfg)
{
}

Vector2 EspModule::GetSmoothedPos(int16_t playerID, const Vector2& rawPos)
{
	ULONGLONG now = GetTickCount64();
	auto it = m_smoothedPosMap.find(playerID);
	if (it == m_smoothedPosMap.end())
	{
		m_smoothedPosMap[playerID] = { rawPos, now };
		return rawPos;
	}

	float dt = (now - it->second.lastUpdateMs) / 1000.0f;
	if (dt <= 0.0001f) return it->second.currentPos;
	if (dt > 0.5f) // Teleport or long gap -> snap
	{
		it->second.currentPos = rawPos;
		it->second.lastUpdateMs = now;
		return rawPos;
	}

	// Exponential smoothing factor based on dt (~25.0f lerp speed for smooth 60fps tracking)
	float alpha = 1.0f - std::exp(-25.0f * dt);
	it->second.currentPos.x += (rawPos.x - it->second.currentPos.x) * alpha;
	it->second.currentPos.y += (rawPos.y - it->second.currentPos.y) * alpha;
	it->second.lastUpdateMs = now;

	return it->second.currentPos;
}

void EspModule::Run() {}


void EspModule::DrawChams()
{
	if (!cfg.bChams) return;

	const auto& ctx = hooks->GetGameContext();

	float screenW = ImGui::GetIO().DisplaySize.x;
	float screenH = ImGui::GetIO().DisplaySize.y;

	ImDrawList* dl = cfg.bChamsThroughWalls
		? ImGui::GetBackgroundDrawList()
		: ImGui::GetForegroundDrawList();
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

	Vector2 localPos = GetSmoothedPos(localNet->playerID, localNet->previousPosition);

	float scale = 67.5f / orthoSize;
	float headRadius = 14.0f * scale;
	float bodyWidth  = 24.0f * scale;
	float bodyHeight = 36.0f * scale;

	std::unordered_set<int16_t> drawn;
	for (auto* target : snapshot)
	{
		if (!target || target->playerIsDead) continue;

		bool isLocal  = (target == localNet);
		bool isFriend = cfg.IsFriend(target->playerID);

		if (cfg.bChamsEnemyOnly && (isFriend || isLocal)) continue;
		if (drawn.count(target->playerID)) continue;
		drawn.insert(target->playerID);

		Vector2 targetPos = GetSmoothedPos(target->playerID, target->previousPosition);
		float dx = targetPos.x - localPos.x;
		float dy = targetPos.y - localPos.y;
		float sx, sy;
		WorldToScreen(dx, dy, screenW, screenH, orthoSize, sx, sy);

		if (sx + bodyWidth < 0 || sx - bodyWidth > screenW) continue;
		if (sy + bodyHeight < 0 || sy - bodyHeight > screenH) continue;

		const float* col = isLocal ? cfg.colChamsLocal.data()
			: (isFriend ? cfg.colChamsFriendly.data() : cfg.colChamsEnemy.data());

		ImU32 fillColor = ImColor(col[0], col[1], col[2], col[3]);
		ImU32 outlineColor = IM_COL32(0, 0, 0, 220);

		// Body Capsule (Bottom)
		ImVec2 bMin(sx - bodyWidth * 0.5f, sy - bodyHeight);
		ImVec2 bMax(sx + bodyWidth * 0.5f, sy);
		dl->AddRectFilled(bMin, bMax, fillColor, bodyWidth * 0.3f);
		dl->AddRect(bMin, bMax, outlineColor, bodyWidth * 0.3f, 0, 2.0f);

		// Head Circle (Top)
		ImVec2 headCenter(sx, sy - bodyHeight - headRadius * 0.7f);
		dl->AddCircleFilled(headCenter, headRadius, fillColor, 32);
		dl->AddCircle(headCenter, headRadius, outlineColor, 32, 2.0f);
	}
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

	Vector2 localPos = GetSmoothedPos(localNet->playerID, localNet->previousPosition);

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		Vector2 targetPos = GetSmoothedPos(target->playerID, target->previousPosition);
		float dx = targetPos.x - localPos.x;
		float dy = targetPos.y - localPos.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist < 0.1f) continue;

		float sx, sy;
		WorldToScreen(dx, dy, ScreenX, ScreenY, orthoSize, sx, sy);

		float scale = 67.5f / orthoSize;
		float rbh   = roundf(60.0f * scale);
		float topY  = sy - rbh;

		char narrowBuf[128] = {};
		if (!ReadIl2CppString(target->playerName, narrowBuf, sizeof(narrowBuf)))
			snprintf(narrowBuf, sizeof(narrowBuf), "Player #%d", target->playerID);

		ImVec2 textSize = ImGui::CalcTextSize(narrowBuf);
		ImU32 colName   = ImColor(cfg.colName[0], cfg.colName[1], cfg.colName[2], cfg.colName[3]);
		dl->AddText(ImVec2(sx - textSize.x * 0.5f, topY - textSize.y - 4.0f), colName, narrowBuf);
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

		float dx   = target->previousPosition.x - localNet->previousPosition.x;
		float dy   = target->previousPosition.y - localNet->previousPosition.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist < 0.1f) continue;

		float angle = std::atan2f(dy, dx);
		float tipX  = cx + std::cosf(angle) * R;
		float tipY  = cy - std::sinf(angle) * R;
		float pa    = angle + 3.14159265f;
		float ha    = 0.4f;

		ImU32 color;
		if (dist < cfg.fNearSnaplineRenderDistance)
			color = IM_COL32(0, 255, 0, 255);
		else if (dist < cfg.fMaxSnaplineRenderDistance)
			color = IM_COL32(255, 0, 0, 255);
		else
			continue;

		dl->AddLine(ImVec2(tipX, tipY),
			ImVec2(tipX + std::cosf(pa - ha) * HL, tipY - std::sinf(pa - ha) * HL),
			color, cfg.fArrowThickness);
		dl->AddLine(ImVec2(tipX, tipY),
			ImVec2(tipX + std::cosf(pa + ha) * HL, tipY - std::sinf(pa + ha) * HL),
			color, cfg.fArrowThickness);
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
		localNet  = ctx.localPlayer->player;
		localPos  = localNet->previousPosition;
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

		float dx   = target->previousPosition.x - localPos.x;
		float dy   = target->previousPosition.y - localPos.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		if (dist > cfg.fMaxSnaplineRenderDistance || dist < 0.1f) continue;

		float sx, sy;
		WorldToScreen(dx, dy, screenW, screenH, orthoSize, sx, sy);

		ImU32 color = dist < cfg.fNearSnaplineRenderDistance
			? IM_COL32(0, 200, 0, 75)
			: IM_COL32(200, 0, 0, 75);

		dl->AddLine(ImVec2(ox, oy), ImVec2(sx, sy), color, 1.0f);
	}
}

void EspModule::DrawBoxes()
{
	if (!cfg.bEsp || !cfg.bBoxes) return;
	if (!cfg.kbEsp.UpdateState() || !cfg.kbBoxes.UpdateState()) return;
	const auto& ctx = hooks->GetGameContext();

	float screenW = ImGui::GetIO().DisplaySize.x;
	float screenH = ImGui::GetIO().DisplaySize.y;

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

	Vector2 localPos = GetSmoothedPos(localNet->playerID, localNet->previousPosition);

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		Vector2 targetPos = GetSmoothedPos(target->playerID, target->previousPosition);
		float dx = targetPos.x - localPos.x;
		float dy = targetPos.y - localPos.y;
		float sx, sy;
		WorldToScreen(dx, dy, screenW, screenH, orthoSize, sx, sy);

		float scale = 67.5f / orthoSize;
		float rbw   = roundf(40.0f * scale);
		float rbh   = roundf(60.0f * scale);

		if (sx + rbw * 0.5f < 0 || sx - rbw * 0.5f > screenW) continue;
		if (sy < 0 || sy - rbh > screenH) continue;

		ImVec2 bMin(roundf(sx - rbw * 0.5f), roundf(sy - rbh));
		ImVec2 bMax(roundf(sx + rbw * 0.5f), roundf(sy));
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

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float sx, sy;
		WorldToScreen(dx, dy, ScreenX, ScreenY, orthoSize, sx, sy);

		if (sx < 0 || sx > ScreenX || sy < 0 || sy > ScreenY) continue;

		int armorLvl = (int)target->currArmorLvl;
		int armorAmt = target->currArmorAmount;
		if (armorLvl <= 0 && armorAmt <= 0) continue;

		char armorBuf[64];
		snprintf(armorBuf, sizeof(armorBuf), "Armor %d (%d)", armorLvl, armorAmt);
		ImVec2 textSize = ImGui::CalcTextSize(armorBuf);
		ImU32 colArmor  = ImColor(cfg.colArmor[0], cfg.colArmor[1], cfg.colArmor[2], cfg.colArmor[3]);
		dl->AddText(ImVec2(sx - textSize.x * 0.5f, sy + 4.0f), colArmor, armorBuf);
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

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (target->grenadeCount <= 0) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		float dx = target->previousPosition.x - localNet->previousPosition.x;
		float dy = target->previousPosition.y - localNet->previousPosition.y;
		float sx, sy;
		WorldToScreen(dx, dy, ScreenX, ScreenY, orthoSize, sx, sy);
		
		if (sx < 0 || sx > ScreenX || sy < 0 || sy > ScreenY) continue;

		char nadeBuf[32];
		snprintf(nadeBuf, sizeof(nadeBuf), "Nades: %d", target->grenadeCount);
		ImVec2 textSize = ImGui::CalcTextSize(nadeBuf);
		ImU32 colNade   = ImColor(cfg.colGrenade[0], cfg.colGrenade[1], cfg.colGrenade[2], cfg.colGrenade[3]);
		float offsetY   = cfg.bArmorEsp ? 20.0f : 4.0f;
		dl->AddText(ImVec2(sx - textSize.x * 0.5f, sy + offsetY), colNade, nadeBuf);
	}
}

void EspModule::DrawSkeleton()
{
	if (!cfg.bEsp || !cfg.bSkeletonEsp) return;

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

	Vector2 localPos = GetSmoothedPos(localNet->playerID, localNet->previousPosition);
	ImU32 colSkeleton = ImColor(cfg.colSkeleton[0], cfg.colSkeleton[1], cfg.colSkeleton[2], cfg.colSkeleton[3]);

	float scale = 67.5f / orthoSize;

	std::unordered_set<int16_t> drawnIDs;
	for (auto* target : snapshot)
	{
		if (!target || target == localNet || target->playerIsDead) continue;
		if (cfg.IsFriend(target->playerID)) continue;
		if (drawnIDs.count(target->playerID)) continue;
		drawnIDs.insert(target->playerID);

		Vector2 targetPos = GetSmoothedPos(target->playerID, target->previousPosition);
		float dx = targetPos.x - localPos.x;
		float dy = targetPos.y - localPos.y;
		float sx, sy;
		WorldToScreen(dx, dy, ScreenX, ScreenY, orthoSize, sx, sy);

		if (sx < -100.0f || sx > ScreenX + 100.0f || sy < -100.0f || sy > ScreenY + 100.0f) continue;

		// Try reading Spine gun bones first if valid
		Bone* gunBones[] = { target->boneGunLow, target->boneGun, target->boneGunLowAiming };
		bool hasGunBones = false;
		ImVec2 prevPoint(sx, sy);

		for (int b = 0; b < 3; b++)
		{
			Bone* bone = gunBones[b];
			if (bone && !IsBadReadPtr(bone, sizeof(Bone)) && (bone->worldX != 0.0f || bone->worldY != 0.0f))
			{
				hasGunBones = true;
				float boneWorldX = targetPos.x + (bone->worldX * 0.02f);
				float boneWorldY = targetPos.y + (bone->worldY * 0.02f);

				float bdx = boneWorldX - localPos.x;
				float bdy = boneWorldY - localPos.y;
				float bx, by;
				WorldToScreen(bdx, bdy, ScreenX, ScreenY, orthoSize, bx, by);

				ImVec2 bonePos(bx, by);
				dl->AddLine(prevPoint, bonePos, colSkeleton, 2.0f);
				dl->AddCircleFilled(bonePos, 3.5f, colSkeleton);
				prevPoint = bonePos;
			}
		}

		// Fallback/Full Spine procedural skeleton projection if no active gun bone offsets
		// Key joint anchors relative to screen position (Head, Neck, Pelvis, Left/Right Arm, Left/Right Leg)
		ImVec2 pelvis(sx, sy);
		ImVec2 neck(sx, sy - 24.0f * scale);
		ImVec2 head(sx, sy - 38.0f * scale);

		ImVec2 lShoulder(sx - 12.0f * scale, sy - 22.0f * scale);
		ImVec2 rShoulder(sx + 12.0f * scale, sy - 22.0f * scale);
		ImVec2 lHand(sx - 16.0f * scale, sy - 10.0f * scale);
		ImVec2 rHand(sx + 16.0f * scale, sy - 10.0f * scale);

		ImVec2 lHip(sx - 8.0f * scale, sy);
		ImVec2 rHip(sx + 8.0f * scale, sy);
		ImVec2 lFoot(sx - 10.0f * scale, sy + 14.0f * scale);
		ImVec2 rFoot(sx + 10.0f * scale, sy + 14.0f * scale);

		// Spine / Torso
		dl->AddLine(pelvis, neck, colSkeleton, 2.0f);
		dl->AddLine(neck, head, colSkeleton, 2.0f);
		dl->AddCircleFilled(head, 6.0f * scale, colSkeleton);

		// Arms
		dl->AddLine(neck, lShoulder, colSkeleton, 2.0f);
		dl->AddLine(lShoulder, lHand, colSkeleton, 2.0f);
		dl->AddCircleFilled(lHand, 2.5f * scale, colSkeleton);

		dl->AddLine(neck, rShoulder, colSkeleton, 2.0f);
		dl->AddLine(rShoulder, rHand, colSkeleton, 2.0f);
		dl->AddCircleFilled(rHand, 2.5f * scale, colSkeleton);

		// Legs
		dl->AddLine(pelvis, lHip, colSkeleton, 2.0f);
		dl->AddLine(lHip, lFoot, colSkeleton, 2.0f);
		dl->AddCircleFilled(lFoot, 2.5f * scale, colSkeleton);

		dl->AddLine(pelvis, rHip, colSkeleton, 2.0f);
		dl->AddLine(rHip, rFoot, colSkeleton, 2.0f);
		dl->AddCircleFilled(rFoot, 2.5f * scale, colSkeleton);
	}
}
