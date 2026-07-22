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