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
constexpr int TAB_MOVEMENT = 5;
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

static void DrawBindGear(const char* id, KeyBind& kb)
{
	ImGui::SameLine(320.0f);
	
	char gearLabel[64];
	if (kb.key != 0)
	{
		const char* modeNames[] = { "Hold", "Toggle", "Always" };
		snprintf(gearLabel, sizeof(gearLabel), " %s [%s] ##gear_%s", GetKeyName(kb.key), modeNames[kb.mode], id);
	}
	else
	{
		snprintf(gearLabel, sizeof(gearLabel), " Bind ##gear_%s", id);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, kb.key != 0 ? ImVec4(0.2f, 0.22f, 0.26f, 0.9f) : ImVec4(0.14f, 0.15f, 0.18f, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.647f, 0.914f, 0.392f, 0.35f));
	ImGui::PushStyleColor(ImGuiCol_Text, kb.key != 0 ? ImVec4(0.647f, 0.914f, 0.392f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f));

	if (ImGui::Button(gearLabel, ImVec2(0, 24)))
	{
		char popupId[64];
		snprintf(popupId, sizeof(popupId), "##bind_pop_%s", id);
		ImGui::OpenPopup(popupId);
	}
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	char popupId[64];
	snprintf(popupId, sizeof(popupId), "##bind_pop_%s", id);
	if (ImGui::BeginPopup(popupId))
	{
		static bool waitingForKey = false;
		static std::string waitingId = "";

		ImGui::Text("Keybind Settings");
		ImGui::Separator();

		if (waitingForKey && waitingId == id)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Press any key (ESC to clear)...");
			for (int vk = 1; vk < 256; vk++)
			{
				if (GetAsyncKeyState(vk) & 0x8000)
				{
					if (vk == VK_ESCAPE)
						kb.key = 0;
					else
						kb.key = vk;
					waitingForKey = false;
					break;
				}
			}
		}
		else
		{
			const char* kName = (kb.key == 0) ? "None" : GetKeyName(kb.key);
			if (ImGui::Button(kName, ImVec2(120, 24)))
			{
				waitingForKey = true;
				waitingId = id;
			}

			const char* modeNames[] = { "Hold", "Toggle", "Always On" };
			for (int i = 0; i < 3; i++)
			{
				if (ImGui::RadioButton(modeNames[i], kb.mode == i))
					kb.mode = i;
			}
		}
		ImGui::EndPopup();
	}
}
static void DrawTabAimbot(Config& cfg)
{
	ImGui::Checkbox("##aimbot_en", &cfg.bAimbot);
	ImGui::SameLine();
	ImGui::Text("Enable Aimbot");
	DrawBindGear("aimbot", cfg.kbAim);

	if (cfg.bAimbot)
	{
		ImGui::Indent(24.0f);
		ImGui::PushItemWidth(200.0f);

		ImGui::Checkbox("Silent Aim (Direct Angle)", &cfg.bSilentAim);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Sets weapon angle directly in memory without moving cursor");

		ImGui::Checkbox("Draw FOV Circle", &cfg.bAimFovCircle);
		if (cfg.bAimFovCircle)
		{
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::ColorEdit4("##colFovCircle", cfg.colFovCircle.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::PopStyleVar();
		}

		ImGui::Spacing();
		ImGui::Text("Target Mode");
		const char* modeItems[] = { "Closest", "Lowest HP", "Highest HP", "Most Kills" };
		ImGui::Combo("##aimbotmode", &cfg.iAimbotMode, modeItems, IM_ARRAYSIZE(modeItems));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("How the aimbot picks its target");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Max Target Distance / FOV");
		ImGui::SliderFloat("##aimdist", &cfg.fAimFov, 10.0f, 2000.0f, "%.0f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Maximum distance to target in world units");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Target Filters");
		ImGui::Checkbox("Only aim when holding weapon", &cfg.bAimOnlyWhenWeapon);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Skip aiming if local player has no gun equipped");
		ImGui::Checkbox("Ignore sniper/bow targets", &cfg.bAimIgnoreSniper);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Skip targets that are holding a sniper or bow");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Checkbox("Target Prediction", &cfg.bAimPrediction);
		if (cfg.bAimPrediction)
		{
			ImGui::Indent(16.0f);
			ImGui::Text("Prediction Scale Factor");
			ImGui::SliderFloat("##predfactor", &cfg.fAimPredictionFactor, 0.1f, 3.0f, "%.2f");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Multiplier for lead distance estimation");

			ImGui::Text("Estimated Bullet Speed");
			ImGui::SliderFloat("##bulletspeed", &cfg.fAimBulletSpeed, 100.0f, 3000.0f, "%.0f");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Projectile travel speed for lead time calculation");
			ImGui::Unindent(16.0f);
		}

		ImGui::PopItemWidth();
		ImGui::Unindent(24.0f);
	}
}

static void DrawTabVisuals(Config& cfg)
{
	ImGui::Checkbox("##esp", &cfg.bEsp);
	ImGui::SameLine();
	ImGui::Text("Enable Visuals");
	DrawBindGear("esp", cfg.kbEsp);

	ImGui::Spacing();
	ImGui::Checkbox("##chams_en", &cfg.bChams);
	ImGui::SameLine();
	ImGui::Text("Chams");
	DrawBindGear("chams", cfg.kbChams);
	if (cfg.bChams)
	{
		ImGui::Indent(24.0f);
		ImGui::Checkbox("Enemy only", &cfg.bChamsEnemyOnly);
		ImGui::Checkbox("Through walls", &cfg.bChamsThroughWalls);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::ColorEdit4("##colChamsEnemy",    cfg.colChamsEnemy.data(),
			ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		ImGui::SameLine(); ImGui::Text("Enemy");
		ImGui::SameLine(0, 16);
		ImGui::ColorEdit4("##colChamsFriendly", cfg.colChamsFriendly.data(),
			ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		ImGui::SameLine(); ImGui::Text("Friendly");
		ImGui::PopStyleVar();
		ImGui::Unindent(24.0f);
	}
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	{
		if (ImGui::BeginChild("##visuals_scroll", ImVec2(0, 360.f), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground))
		{
			ImGui::Indent(24.0f);
			ImGui::Checkbox("Boxes", &cfg.bBoxes);
			DrawBindGear("boxes", cfg.kbBoxes);

			ImGui::Checkbox("Skeleton ESP", &cfg.bSkeletonEsp);
			DrawBindGear("skeleton", cfg.kbSkeleton);

			ImGui::Checkbox("Snap Lines", &cfg.bSnaplines);
			DrawBindGear("snaplines", cfg.kbSnaplines);

			ImGui::Checkbox("Arrows", &cfg.bArrows);
			DrawBindGear("arrows", cfg.kbArrows);

			ImGui::Checkbox("Names", &cfg.bNames);
			DrawBindGear("names", cfg.kbNames);

			ImGui::Checkbox("Armor ESP", &cfg.bArmorEsp);
			DrawBindGear("armoresp", cfg.kbArmorEsp);

			ImGui::Checkbox("Grenade ESP", &cfg.bGrenadeEsp);
			DrawBindGear("nadeesp", cfg.kbGrenadeEsp);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Colors");
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::ColorEdit4("##colBox", cfg.colBox.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
			ImGui::SameLine(); ImGui::Text("Box");
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##colSkeleton", cfg.colSkeleton.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
			ImGui::SameLine(); ImGui::Text("Skeleton");

			ImGui::ColorEdit4("##colName", cfg.colName.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
			ImGui::SameLine(); ImGui::Text("Name");
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##colArmor", cfg.colArmor.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
			ImGui::SameLine(); ImGui::Text("Armor");

			ImGui::ColorEdit4("##colNade", cfg.colGrenade.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
			ImGui::SameLine(); ImGui::Text("Nade");
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##colFovCircle", cfg.colFovCircle.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(); ImGui::Text("FOV Circle");
			ImGui::PopStyleVar();
			ImGui::Separator();

			ImGui::Checkbox("##zoom", &cfg.bZoomOverride);
			ImGui::SameLine();
			ImGui::Text("Zoom Override");
			if (cfg.bZoomOverride)
			{
				ImGui::Indent(24.0f);
				ImGui::PushItemWidth(200.0f);
				ImGui::SliderFloat("##zoomval", &cfg.fZoomOverrideValue, 15.0f, 500.0f, "%.1f");
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

static void DrawTabPlayers(GameContext& ctx, Config& cfg)
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
	ImGui::SameLine(180.0f);
	ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "Lmb - friend add");

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

			bool isFriend = cfg.IsFriend(player->playerID);

			ImVec4 color;
			if (player->playerIsDead)
				color = ImVec4(0.6f, 0.2f, 0.2f, 1.0f);
			else if (isFriend)
				color = ImVec4(0.2f, 0.85f, 1.0f, 1.0f);
			else
				color = ImVec4(0.647f, 0.914f, 0.392f, 1.0f);

			char narrowName[128] = "???";
			if (player->playerName && !IsBadReadPtr(player->playerName, 0x18))
			{
				int32_t len = *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(player->playerName) + 0x10);
				if (len > 0 && len < 256)
				{
					const wchar_t* wStr = reinterpret_cast<const wchar_t*>(reinterpret_cast<const char*>(player->playerName) + 0x14);
					if (!IsBadReadPtr(wStr, len * sizeof(wchar_t)))
					{
						int i = 0;
						for (; i < 127 && i < len && wStr[i]; i++)
							narrowName[i] = (wStr[i] < 128) ? (char)wStr[i] : '?';
						narrowName[i] = '\0';
					}
				}
			}

			char labelBuf[256];
			snprintf(labelBuf, sizeof(labelBuf), "%s%s##p_%d", isFriend ? "(Friend) " : "", narrowName, player->playerID);

			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::Selectable(labelBuf, isFriend, ImGuiSelectableFlags_AllowOverlap);
			ImGui::PopStyleColor();

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				cfg.ToggleFriend(player->playerID);
			}

			ImGui::SameLine(240.0f);
			ImGui::Text("HP: %.0f/%.0f", player->playerHP, player->playerHPMax);
			ImGui::SameLine(360.0f);
			ImGui::Text("ID: %d", player->playerID);
		}
	}
	ImGui::EndChild();
}

static void DrawTabMisc(Config& cfg)
{
	// ---- Rapid Fire ----
	ImGui::Checkbox("##rapidfire_en", &cfg.bRapidFire);
	ImGui::SameLine();
	ImGui::Text("Rapid Fire");
	DrawBindGear("rapidfire", cfg.kbRapidFire);
	if (cfg.bRapidFire)
	{
		ImGui::Indent(24.0f);
		ImGui::PushItemWidth(200.0f);
		ImGui::Text("Fire Rate Multiplier (0 = instant)");
		ImGui::SliderFloat("##rfmult", &cfg.fRapidFireMultiplier, 0.0f, 1.0f, "%.2f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("0 = instant re-fire, 1 = original rate");
		ImGui::PopItemWidth();
		ImGui::Unindent(24.0f);
	}

	ImGui::Spacing();

	// ---- Feature Indicator ----
	ImGui::Checkbox("##featurelist_en", &cfg.bFeatureList);
	ImGui::SameLine();
	ImGui::Text("Feature Indicator");
	DrawBindGear("featurelist", cfg.kbFeatureList);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// ---- Velocity Indicator ----
	ImGui::Checkbox("##velocity_en", &cfg.bVelocityIndicator);
	ImGui::SameLine();
	ImGui::Text("Velocity Indicator");
	DrawBindGear("velocity", cfg.kbVelocityIndicator);
}

static void DrawTabMovement(Config& cfg)
{
	ImGui::Checkbox("##antibanana_en", &cfg.bAntiBanana);
	ImGui::SameLine();
	ImGui::Text("Anti-Banana / Auto Stun Reset");
	DrawBindGear("antibanana", cfg.kbAntiBanana);
	ImGui::Spacing();

	ImGui::Checkbox("##infroll_en", &cfg.bInfiniteRoll);
	ImGui::SameLine();
	ImGui::Text("Infinite Roll (No Cooldown)");
	DrawBindGear("infroll", cfg.kbInfiniteRoll);
	ImGui::Spacing();

	ImGui::Checkbox("##bhop_en", &cfg.bBhop);
	ImGui::SameLine();
	ImGui::Text("Jump Boost");
	DrawBindGear("Jb", cfg.kbBhop);
	ImGui::Spacing();

	ImGui::Checkbox("##vehicle_boost", &cfg.bVehicleBoost);
	ImGui::SameLine();
	ImGui::Text("Emu & Hamster Ball Speed Boost");
	ImGui::Spacing();

	ImGui::Checkbox("##fast_parachute", &cfg.bFastParachute);
	ImGui::SameLine();
	ImGui::Text("Fast Parachute Drop");
	DrawBindGear("parachute", cfg.kbFastParachute);

	if (cfg.bFastParachute)
	{
		ImGui::Indent(24.0f);
		ImGui::PushItemWidth(200.0f);
		ImGui::SliderFloat("##dropspeed", &cfg.fParachuteDropSpeed, 1.0f, 50.0f, "%.1f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Parachute drop speed multiplier");
		ImGui::PopItemWidth();
		ImGui::Unindent(24.0f);
	}

	ImGui::Spacing();

	ImGui::Checkbox("##anim_disable", &cfg.bDisableMoveAnim);
	ImGui::SameLine();
	ImGui::Text("Disable Movement Animation");
	DrawBindGear("moveanim", cfg.kbDisableMoveAnim);

	ImGui::Spacing();

	ImGui::Checkbox("##vehicle_fly", &cfg.bVehicleFly);
	ImGui::SameLine();
	ImGui::Text("Vehicle / Hamster Ball Fly");
	DrawBindGear("vehiclefly", cfg.kbVehicleFly);

	if (cfg.bVehicleFly)
	{
		ImGui::Indent(24.0f);
		ImGui::PushItemWidth(200.0f);
		ImGui::SliderFloat("Fly Speed", &cfg.fVehicleFlySpeed, 5.0f, 200.0f, "%.1f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Speed of flying in vehicle or hamster ball");
		ImGui::PopItemWidth();
		ImGui::Unindent(24.0f);
	}
}

static void DrawTabConfig(Config& cfg)
{
	static std::vector<std::string> configFiles;
	static int selectedConfig = -1;
	static char newConfigName[64] = "my_config";
	static std::string statusMsg = "";
	static ImVec4 statusColor = ImVec4(0.647f, 0.914f, 0.392f, 1.0f);
	static bool initialized = false;

	auto RefreshConfigs = []() {
		configFiles.clear();
		WIN32_FIND_DATAA findData;
		HANDLE hFind = FindFirstFileA("*.json", &findData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					configFiles.push_back(findData.cFileName);
				}
			} while (FindNextFileA(hFind, &findData));
			FindClose(hFind);
		}
	};

	if (!initialized)
	{
		RefreshConfigs();
		initialized = true;
	}

	ImGui::Text("Config List");
	ImGui::Indent(8.0f);

	if (ImGui::BeginChild("##config_list_child", ImVec2(340, 110), true))
	{
		if (configFiles.empty())
		{
			ImGui::TextDisabled("No configs found ()");
		}
		else
		{
			for (int i = 0; i < (int)configFiles.size(); i++)
			{
				bool isSelected = (selectedConfig == i);
				if (ImGui::Selectable(configFiles[i].c_str(), isSelected))
				{
					selectedConfig = i;
				}
			}
		}
	}
	ImGui::EndChild();

	ImGui::Spacing();

	if (ImGui::Button("Refresh", ImVec2(75, 24)))
	{
		RefreshConfigs();
		statusMsg = "Refreshed list!";
		statusColor = ImVec4(0.647f, 0.914f, 0.392f, 1.0f);
	}

	ImGui::SameLine();

	if (ImGui::Button("Load Selected", ImVec2(105, 24)))
	{
		if (selectedConfig >= 0 && selectedConfig < (int)configFiles.size())
		{
			if (cfg.LoadFromFile(configFiles[selectedConfig]))
			{
				statusMsg = "Loaded: " + configFiles[selectedConfig];
				statusColor = ImVec4(0.3f, 0.9f, 0.3f, 1.0f);
			}
			else
			{
				statusMsg = "Failed to load!";
				statusColor = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
			}
		}
		else
		{
			statusMsg = "Select a config first!";
			statusColor = ImVec4(0.9f, 0.7f, 0.2f, 1.0f);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Save Selected", ImVec2(105, 24)))
	{
		if (selectedConfig >= 0 && selectedConfig < (int)configFiles.size())
		{
			if (cfg.SaveToFile(configFiles[selectedConfig]))
			{
				statusMsg = "Saved: " + configFiles[selectedConfig];
				statusColor = ImVec4(0.3f, 0.9f, 0.3f, 1.0f);
			}
			else
			{
				statusMsg = "Failed to save!";
				statusColor = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
			}
		}
	}

	ImGui::Spacing();

	ImGui::PushItemWidth(180.0f);
	ImGui::InputText("##newcfgname", newConfigName, sizeof(newConfigName));
	ImGui::PopItemWidth();	

	ImGui::SameLine();

	if (ImGui::Button("Create New", ImVec2(90, 24)))
	{
		std::string name = newConfigName;
		if (name.find(".json") == std::string::npos)
			name += ".json";

		if (cfg.SaveToFile(name))
		{
			statusMsg = "Created: " + name;
			statusColor = ImVec4(0.3f, 0.9f, 0.3f, 1.0f);
			RefreshConfigs();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(55, 24)))
	{
		cfg.Reset();
		statusMsg = "Reset to defaults!";
		statusColor = ImVec4(0.9f, 0.7f, 0.2f, 1.0f);
	}

	if (!statusMsg.empty())
	{
		ImGui::Spacing();
		ImGui::TextColored(statusColor, "%s", statusMsg.c_str());
	}

	ImGui::Unindent(8.0f);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Controls");
	ImGui::Indent(8.0f);
	ImGui::TextColored(ImVec4(0.647f, 0.914f, 0.392f, 1.0f), "Insert Key");
	ImGui::SameLine();
	ImGui::Text("Toggle menu");
	ImGui::Unindent(8.0f);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

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

		float sidebarW = 140.0f;

		ImGui::SetCursorPosY(titleH + 4.0f);

		ImGui::Columns(2, "##layout", false);
		ImGui::SetColumnWidth(0, sidebarW);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

		ImGui::Spacing();
		DrawTabButton("Aimbot", TAB_AIMBOT);
		DrawTabButton("Visuals", TAB_VISUALS);
		DrawTabButton("Movement", TAB_MOVEMENT);
		DrawTabButton("Misc", TAB_MISC);
		DrawTabButton("Players List", TAB_PLAYERS);
		DrawTabButton("Config", TAB_CONFIG);

		ImGui::PopStyleVar(2);

		ImGui::NextColumn();
		ImGui::SetCursorPosY(titleH + 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
		ImGui::BeginGroup();

		if (g_activeTab == TAB_AIMBOT)
			DrawTabAimbot(cfg);
		else if (g_activeTab == TAB_MISC)
			DrawTabMisc(cfg);
		else if (g_activeTab == TAB_MOVEMENT)
			DrawTabMovement(cfg);
		else if (g_activeTab == TAB_PLAYERS)
			DrawTabPlayers(ctx, cfg);
		else if (g_activeTab == TAB_VISUALS)
			DrawTabVisuals(cfg);
		else if (g_activeTab == TAB_CONFIG)
			DrawTabConfig(cfg);

		ImGui::EndGroup();
		ImGui::PopStyleVar();
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
