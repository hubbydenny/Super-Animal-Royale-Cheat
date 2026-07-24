#include "Menu.hpp"
#include <imgui.h>
#include <vector>
#include <Windows.h>


static int g_activeTab = 0;
constexpr int TAB_VISUALS = 3;
constexpr int TAB_PLAYERS = 1;
constexpr int TAB_MISC    = 2;
constexpr int TAB_AIMBOT  = 0;
constexpr int TAB_CONFIG  = 4;
constexpr ImVec4 AccentColor = { 165, 233, 100, 255 };

static const char* GetKeyName(int vk)
{
	switch (vk)
	{
	case VK_LBUTTON: return "LMB";
	case VK_RBUTTON: return "RMB";
	case VK_MBUTTON: return "MMB";
	case VK_XBUTTON1: return "Mouse4";
	case VK_XBUTTON2: return "Mouse5";
	case VK_LSHIFT: return "LShift";
	case VK_RSHIFT: return "RShift";
	case VK_LCONTROL: return "LCtrl";
	case VK_RCONTROL: return "RCtrl";
	case VK_LMENU: return "LAlt";
	case VK_RMENU: return "RAlt";
	case VK_SPACE: return "Space";
	case VK_TAB: return "Tab";
	case VK_CAPITAL: return "CapsLock";
	case VK_RETURN: return "Enter";
	case VK_BACK: return "Backspace";
	case VK_ESCAPE: return "Escape";
	case VK_F1: return "F1";
	case VK_F2: return "F2";
	case VK_F3: return "F3";
	case VK_F4: return "F4";
	case VK_F5: return "F5";
	case VK_F6: return "F6";
	case VK_F7: return "F7";
	case VK_F8: return "F8";
	case VK_F9: return "F9";
	case VK_F10: return "F10";
	case VK_F11: return "F11";
	case VK_F12: return "F12";
	case 0x57: return "W";
	case 0x41: return "A";
	case 0x53: return "S";
	case 0x44: return "D";
	case 0x51: return "Q";
	case 0x45: return "E";
	case 0x52: return "R";
	case 0x46: return "F";
	case 0x5A: return "Z";
	case 0x58: return "X";
	case 0x43: return "C";
	case 0x56: return "V";
	default: return "???";
	}
}

static void DrawTabAimbot(Config& cfg)
{
	ImGui::Checkbox("##aimbot_en", &cfg.bAimbot);
	ImGui::SameLine();
	ImGui::Text("Enable Aimbot");

	if (cfg.bAimbot)
	{
		ImGui::Indent(24.0f);

		ImGui::PushItemWidth(200.0f);
		ImGui::SliderFloat("##aimfov", &cfg.fAimFov, 10.0f, 2000.0f, "%.0f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Max aim distance");
		ImGui::PopItemWidth();

		const char* modeNames[] = { "Hold", "Toggle", "Always On" };

		static bool waitingForKey = false;

		if (waitingForKey)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Press any key...");

			for (int vk = 2; vk < 256; vk++)
			{
				if (vk == VK_ESCAPE)
				{
					if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
						waitingForKey = false;
					continue;
				}
				if (GetAsyncKeyState(vk) & 0x8000)
				{
					cfg.iAimKey = vk;
					waitingForKey = false;
					break;
				}
			}
		}
		else
		{
			const char* keyName = GetKeyName(cfg.iAimKey);
			char keyBuf[64];
			snprintf(keyBuf, sizeof(keyBuf), "[%s] - %s", keyName, modeNames[cfg.iAimMode]);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.30f, 1.0f));
			ImGui::Button(keyBuf, ImVec2(200.0f, 24.0f));
			ImGui::PopStyleColor(2);

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				waitingForKey = true;

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup("##aim_mode_popup");

			if (ImGui::BeginPopup("##aim_mode_popup"))
			{
				for (int i = 0; i < 3; i++)
				{
					bool selected = (cfg.iAimMode == i);
					if (ImGui::Selectable(modeNames[i], selected))
						cfg.iAimMode = i;
				}
				ImGui::EndPopup();
			}
		}

		ImGui::Unindent(24.0f);
	}
}

static void DrawTabButton(const char* label, int tab_id)
{
	bool isActive = (g_activeTab == tab_id);
	if (isActive)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.647f, 0.914f, 0.392f, 1.0f));

	if (ImGui::Selectable(label, isActive, ImGuiSelectableFlags_None, ImVec2(120, 30)))
		g_activeTab = tab_id;

	if (isActive)
		ImGui::PopStyleColor();
}

static void DrawTabVisuals(Config& cfg)
{
	ImGui::Checkbox("##esp", &cfg.bEsp);
	ImGui::SameLine();
	ImGui::Text("Enable Visuals");
	
	if (cfg.bEsp)
	{
		if (ImGui::BeginChild("##visuals_scroll", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground))
		{
			ImGui::Indent(24.0f);
			ImGui::Checkbox("Boxes", &cfg.bBoxes);
			ImGui::Checkbox("Snap Lines", &cfg.bSnaplines);
			ImGui::Checkbox("Arrows", &cfg.bArrows);
			ImGui::Checkbox("Names", &cfg.bNames);
		//	ImGui::Checkbox("Circle", &cfg.bCircle);
			ImGui::Checkbox("##zoom", &cfg.bZoomOverride);
			ImGui::SameLine();
			ImGui::Text("Zoom Override");
			if (cfg.bZoomOverride)
			{
				ImGui::Indent(24.0f);
				ImGui::PushItemWidth(200.0f);
				ImGui::SliderFloat("##zoomval", &cfg.fZoomOverrideValue, 67.6f, 400.f, "%.1f");
				ImGui::PopItemWidth();
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Player Fov");
				ImGui::Unindent(24.0f);
			}
			if (cfg.bArrows || cfg.bSnaplines)
			{
				ImGui::PushItemWidth(200.0f);
				if (cfg.bArrows)
				{
					ImGui::SliderFloat("##arrowsradius", &cfg.fArrowsRadius, 30.0f, 300.0f, "%.0f");
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Arrows / Names radius");
					ImGui::SliderFloat("##arrowthick", &cfg.fArrowThickness, 1.0f, 5.0f, "%.1f");
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Arrow line thickness");
				}
				if (cfg.bSnaplines)
				{
					ImGui::SliderFloat("##snaplen", &cfg.fSnaplineLength, 50.0f, 1500.0f, "%.0f");
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Snapline length");
				}
				ImGui::SliderFloat("##snapmax", &cfg.fMaxSnaplineRenderDistance, 0.0f, 5000.0f, "%.f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Max render distance");
				ImGui::SliderFloat("##snapnear", &cfg.fNearSnaplineRenderDistance, 0.0f, 5000.0f, "%.f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Near render distance");
				ImGui::PopItemWidth();
			}
			ImGui::Unindent(24.0f);
		}
		ImGui::EndChild();
	}
	ImGui::Spacing();
}

static void DrawTabPlayers(GameContext& ctx)
{
	std::vector<NetworkPlayer*> snapshot;
	LocalPlayerScript* localPlayer = nullptr;
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		snapshot.assign(ctx.players.begin(), ctx.players.end());
		localPlayer = ctx.localPlayer;
	}

	int aliveCount = 0;
	for (auto* player : snapshot)
	{
		if (player && !player->playerIsDead)
			aliveCount++;
	}

	ImGui::Text("Alive: %d / %d", aliveCount, (int)snapshot.size());

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (snapshot.empty())
	{
		ImGui::TextDisabled("Waiting for players...");
		return;
	}

	if (ImGui::BeginChild("##playerlist", ImVec2(0, 0), ImGuiChildFlags_Border))
	{
		for (auto* player : snapshot)
		{
			if (!player) continue;

			ImVec4 color = player->playerIsDead
				? ImVec4(0.6f, 0.2f, 0.2f, 1.0f)
				: ImVec4(0.647f, 0.914f, 0.392f, 1.0f);

			char narrowName[128] = "???";
			if (player->playerName)
			{
				const wchar_t* wStr = (const wchar_t*)((const char*)player->playerName + 0x14);
				int i = 0;
				for (; i < 127 && wStr[i]; i++)
					narrowName[i] = (char)wStr[i];
				narrowName[i] = '\0';
			}

			ImGui::TextColored(color, "%s", narrowName);
			ImGui::SameLine(200.0f);
			ImGui::Text("HP: %.0f/%.0f", player->playerHP, player->playerHPMax);
			ImGui::SameLine(320.0f);
			ImGui::Text("ID: %d", player->playerID);
		}
	}
	ImGui::EndChild();
}

static void DrawTabMisc(Config& cfg)
{
	ImGui::TextColored(ImVec4(0.450f, 0.475f, 0.500f, 1.0f), "Add more features here");
}

static void DrawTabConfig(Config& cfg)
{
	ImGui::Text("Controls");
	ImGui::Indent(8.0f);
	ImGui::TextColored(ImVec4(0.647f, 0.914f, 0.392f, 1.0f), "Insert Key");
	ImGui::SameLine();
	ImGui::Text("Toggle menu");
	ImGui::Unindent(8.0f);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	/**/
	ImGui::Text("Debug");
	ImGui::Indent(8.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
	if (ImGui::Button("Unload Cheat", ImVec2(180, 30)))
		cfg.bUnloadCheat = true;
	ImGui::PopStyleColor(2);

	ImGui::Unindent(8.0f);
}

void DrawMenu(Config& cfg, GameContext& ctx, bool menuOpen)
{
	if (!menuOpen) return;

	ImGui::SetNextWindowSize({680.0f, 460.0f});
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("##hack_main", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::PopStyleVar();

		ImVec2 wPos = ImGui::GetWindowPos();
		ImVec2 wSize = ImGui::GetWindowSize();
		ImDrawList* dl = ImGui::GetWindowDrawList();

		float titleH = 40.0f;
		dl->AddRectFilled(wPos, ImVec2(wPos.x + wSize.x, wPos.y + titleH),
			ImColor(20, 22, 24), 8.0f, ImDrawFlags_RoundCornersTop);
		dl->AddText(ImVec2(wPos.x + 16, wPos.y + 10),
			ImColor(0.647f, 0.914f, 0.392f, 1.0f), "Larparius");

		dl->AddRectFilled(
			ImVec2(wPos.x, wPos.y + titleH - 1),
			ImVec2(wPos.x + wSize.x, wPos.y + titleH),
			ImColor(0.647f, 0.914f, 0.392f, 0.5f));

		ImGui::SetCursorPosY(titleH + 4.0f);

		float sidebarW = 140.0f;
		ImGui::Columns(2, "##layout", false);
		ImGui::SetColumnWidth(0, sidebarW);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

		ImGui::Spacing();
		DrawTabButton("Aimbot", TAB_AIMBOT);
		DrawTabButton("Visuals", TAB_VISUALS);
		DrawTabButton("Misc", TAB_MISC);
		DrawTabButton("Players List", TAB_PLAYERS);
		DrawTabButton("Config", TAB_CONFIG);

		ImGui::PopStyleVar(2);

		ImGui::NextColumn();
		ImGui::SetCursorPosY(titleH + 8.0f);
		ImGui::BeginGroup();

		if (g_activeTab == TAB_AIMBOT)
			DrawTabAimbot(cfg);
		else if (g_activeTab == TAB_MISC)
			DrawTabMisc(cfg);
		else if (g_activeTab == TAB_PLAYERS)
			DrawTabPlayers(ctx);
		else if (g_activeTab == TAB_VISUALS)
			DrawTabVisuals(cfg);
		else if (g_activeTab == TAB_CONFIG)
			DrawTabConfig(cfg);

		ImGui::EndGroup();
		ImGui::Columns(1);
	}
	else
	{
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

void InitMenuStyles()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	style.WindowRounding = 8.0f;
	style.FrameRounding = 5.0f;
	style.PopupRounding = 5.0f;
	style.ScrollbarRounding = 8.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 7.0f;
	
	style.WindowPadding = ImVec2(16, 16);
	style.FramePadding = ImVec2(12, 6);

	style.ItemSpacing = ImVec2(8, 8);
	style.ItemInnerSpacing = ImVec2(6, 6);

	style.ScrollbarSize = 14.0f;
	style.GrabMinSize = 10.0f;
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;

	ImVec4 bg         = { 0.055f, 0.059f, 0.063f, 1.00f };
	ImVec4 bgLight    = { 0.075f, 0.078f, 0.082f, 1.00f };
	ImVec4 bgFrame    = { 0.098f, 0.102f, 0.106f, 1.00f };
	ImVec4 border     = { 0.145f, 0.149f, 0.153f, 1.00f };
	ImVec4 accent     = { 0.647f, 0.914f, 0.392f, 1.00f };
	ImVec4 accentDim  = { 0.647f, 0.914f, 0.392f, 0.30f };
	ImVec4 text       = { 0.850f, 0.875f, 0.900f, 1.00f };
	ImVec4 textDim    = { 0.450f, 0.475f, 0.500f, 1.00f };

	colors[ImGuiCol_WindowBg]              = bg;
	colors[ImGuiCol_ChildBg]               = ImVec4(0, 0, 0, 0);
	colors[ImGuiCol_PopupBg]               = bgLight;
	colors[ImGuiCol_Border]                = border;
	colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
	colors[ImGuiCol_FrameBg]               = bgFrame;
	colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.12f, 0.13f, 0.14f, 1.0f);
	colors[ImGuiCol_FrameBgActive]         = ImVec4(0.15f, 0.16f, 0.17f, 1.0f);
	colors[ImGuiCol_TitleBg]               = bg;
	colors[ImGuiCol_TitleBgActive]         = bg;
	colors[ImGuiCol_TitleBgCollapsed]      = bg;
	colors[ImGuiCol_ScrollbarBg]           = ImVec4(0, 0, 0, 0);
	colors[ImGuiCol_ScrollbarGrab]         = accentDim;
	colors[ImGuiCol_ScrollbarGrabHovered]  = accent;
	colors[ImGuiCol_ScrollbarGrabActive]   = accent;

	colors[ImGuiCol_CheckMark]             = accent;

	colors[ImGuiCol_SliderGrab]            = accent;
	colors[ImGuiCol_SliderGrabActive]      = accent;

	colors[ImGuiCol_Button]                = accentDim;
	colors[ImGuiCol_ButtonHovered]         = ImVec4(0.647f, 0.914f, 0.392f, 0.50f);
	colors[ImGuiCol_ButtonActive]          = ImVec4(0.647f, 0.914f, 0.392f, 0.70f);

	colors[ImGuiCol_Header]                = accentDim;
	colors[ImGuiCol_HeaderHovered]         = ImVec4(0.647f, 0.914f, 0.392f, 0.50f);
	colors[ImGuiCol_HeaderActive]          = ImVec4(0.647f, 0.914f, 0.392f, 0.70f);

	colors[ImGuiCol_Tab]                   = bgLight;
	colors[ImGuiCol_TabHovered]            = accentDim;
	colors[ImGuiCol_TabActive]             = ImVec4(0.10f, 0.11f, 0.12f, 1.0f);

	colors[ImGuiCol_Separator]             = border;
	colors[ImGuiCol_SeparatorHovered]      = accentDim;
	colors[ImGuiCol_SeparatorActive]       = accent;

	colors[ImGuiCol_ResizeGrip]            = ImVec4(0, 0, 0, 0);
	colors[ImGuiCol_ResizeGripHovered]     = accentDim;
	colors[ImGuiCol_ResizeGripActive]      = accentDim;

	colors[ImGuiCol_Text]                  = text;
	colors[ImGuiCol_TextDisabled]          = textDim;
}
