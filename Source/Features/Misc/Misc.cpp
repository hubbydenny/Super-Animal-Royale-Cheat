#include "Misc.hpp"
#include <spdlog/spdlog.h>
#include "../../Core/Hooks/HooksManager.hpp"
#include "../../Core/GameStructs/GameStructs.hpp"
#include <imgui.h>

extern GameContext* gpCtx;

//main
MiscModule::MiscModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg)
	: ModuleBase(MISC_MODULE_NAME, modules, hooks, cfg)
{
};

void MiscModule::Run()
{
}
void MiscModule::DrawWatermark() {
	if (!gpCtx) return;
	ImDrawList* DrawList = ImGui::GetForegroundDrawList();
	if (DrawList) {
		float fps = ImGui::GetIO().Framerate;
		char buf[64];
		snprintf(buf, sizeof(buf), "larparius | fps: %.0f", fps);
		ImVec2 pos(1760.0f, 20.0f);
		ImVec2 textSize = ImGui::CalcTextSize(buf);
		ImVec2 rectMin = ImVec2(pos.x - 12.0f, pos.y - 6.0f);
		ImVec2 rectMax = ImVec2(pos.x + textSize.x + 12.0f, pos.y + textSize.y + 6.0f);
		DrawList->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, 255), 0.0f);
		DrawList->AddText(pos, IM_COL32(165, 233, 100, 200), buf);
	}
}

void MiscModule::DrawFeatureIndicator()
{
	if (!cfg.bFeatureList || !cfg.kbFeatureList.UpdateState()) return;

	struct ActiveFeature {
		std::string name;
		std::string details;
	};

	std::vector<ActiveFeature> features;

	if (cfg.bAimbot && cfg.kbAim.UpdateState()) {
		features.push_back({ "Aimbot", "ON" });
	}

	if (cfg.bEsp && cfg.kbEsp.UpdateState()) {
		std::string espSub;
		if (cfg.bBoxes && cfg.kbBoxes.UpdateState()) espSub += "Box ";
		if (cfg.bNames && cfg.kbNames.UpdateState()) espSub += "Name ";
		if (cfg.bSnaplines && cfg.kbSnaplines.UpdateState()) espSub += "Snap ";
		if (cfg.bArrows && cfg.kbArrows.UpdateState()) espSub += "Arrow ";
		if (cfg.bArmorEsp && cfg.kbArmorEsp.UpdateState()) espSub += "Armor ";
		if (cfg.bGrenadeEsp && cfg.kbGrenadeEsp.UpdateState()) espSub += "Nade ";

		if (espSub.empty()) espSub = "ON";
		features.push_back({ "Visuals (ESP)", espSub });
	}

	if (cfg.bBhop && cfg.kbBhop.UpdateState()) {
		features.push_back({ "Bunnyhop", "ON" });
	}

	if (cfg.bFastParachute && cfg.kbFastParachute.UpdateState()) {
		features.push_back({ "Fast Parachute", "ON" });
	}

	if (cfg.bVehicleFly && cfg.kbVehicleFly.UpdateState()) {
		features.push_back({ "Vehicle Fly", "ON" });
	}

	if (cfg.bZoomOverride) {
		features.push_back({ "Zoom Hack", "ON" });
	}

	if (cfg.bDisableMoveAnim && cfg.kbDisableMoveAnim.UpdateState()) {
		features.push_back({ "No Move Anim", "ON" });
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(190.0f, 0.0f), ImVec2(340.0f, 600.0f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.11f, 0.85f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.647f, 0.914f, 0.392f, 0.60f));

	if (ImGui::Begin("##ActiveFeaturesOverlay", nullptr, flags))
	{
		ImGui::TextColored(ImVec4(0.647f, 0.914f, 0.392f, 1.0f), "Active Features");
		ImGui::Separator();
		ImGui::Spacing();

		if (features.empty())
		{
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No active features");
		}
		else
		{
			for (const auto& feat : features)
			{
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "%s", feat.name.c_str());
				ImGui::SameLine(140.0f);
				ImGui::TextColored(ImVec4(0.647f, 0.914f, 0.392f, 0.9f), "[%s]", feat.details.c_str());
			}
		}
	}
	ImGui::End();

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
}

void MiscModule::DrawVelocityIndicator()
{
	if (!cfg.bVelocityIndicator || !cfg.kbVelocityIndicator.UpdateState()) return;
	if (!gpCtx) return;

	NetworkPlayer* localNet = nullptr;
	{
		std::lock_guard<std::mutex> lock(gpCtx->mtx);
		if (!gpCtx->localPlayer || !gpCtx->localPlayer->player) return;
		localNet = gpCtx->localPlayer->player;
	}
	if (!localNet) return;

	static Vector2 s_lastPos = { 0.0f, 0.0f };
	static float s_smoothedVel = 0.0f;
	static ULONGLONG s_lastTimeMs = 0;

	ULONGLONG now = GetTickCount64();
	if (s_lastTimeMs == 0)
	{
		s_lastTimeMs = now;
		s_lastPos = localNet->previousPosition;
		return;
	}

	float dt = (now - s_lastTimeMs) / 1000.0f;
	if (dt >= 0.01f)
	{
		s_lastTimeMs = now;
		Vector2 curPos = localNet->previousPosition;
		float dx = curPos.x - s_lastPos.x;
		float dy = curPos.y - s_lastPos.y;
		float dist = std::sqrtf(dx * dx + dy * dy);
		float rawVel = (dist / dt) * 15.0f;
		s_smoothedVel = s_smoothedVel * 0.75f + rawVel * 0.25f;
		s_lastPos = curPos;
	}

	ImDrawList* dl = ImGui::GetForegroundDrawList();
	if (!dl) return;

	float screenW = ImGui::GetIO().DisplaySize.x;
	float screenH = ImGui::GetIO().DisplaySize.y;

	char velText[32];
	snprintf(velText, sizeof(velText), "%d", (int)s_smoothedVel);

	ImFont* font = ImGui::GetFont();
	float fontSize = 34.0f;
	ImVec2 textSize = font ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, velText) : ImGui::CalcTextSize(velText);

	float posX = (screenW - textSize.x) * 0.5f;
	float posY = screenH * 0.72f;

	float barW = textSize.x + 24.0f;
	float barH = 3.0f;
	ImVec2 barMin(screenW * 0.5f - barW * 0.5f, posY + textSize.y + 4.0f);
	ImVec2 barMax(screenW * 0.5f + barW * 0.5f, posY + textSize.y + 4.0f + barH);
	dl->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, 220));

	for (int xo = -2; xo <= 2; xo++)
	{
		for (int yo = -2; yo <= 2; yo++)
		{
			if (xo == 0 && yo == 0) continue;
			if (font)
				dl->AddText(font, fontSize, ImVec2(posX + xo, posY + yo), IM_COL32(0, 0, 0, 230), velText);
			else
				dl->AddText(ImVec2(posX + xo, posY + yo), IM_COL32(0, 0, 0, 230), velText);
		}
	}

	if (font)
		dl->AddText(font, fontSize, ImVec2(posX, posY), IM_COL32(255, 255, 255, 255), velText);
	else
		dl->AddText(ImVec2(posX, posY), IM_COL32(255, 255, 255, 255), velText);
}