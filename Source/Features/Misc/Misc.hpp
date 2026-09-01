#pragma once

#include "../../Core/Modules/ModuleBase.hpp"

constexpr const char* MISC_MODULE_NAME = "MISC";

class MiscModule final : public ModuleBase
{
public:
	explicit MiscModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg);

	void Run();
	void Draw();
	void DrawWatermark();
	void DrawFeatureIndicator();
	void DrawVelocityIndicator();

	// RapidFire: zeroes fire-rate cooldown on local player's active weapon each frame
	void RunRapidFire();

	// Weapon boolean update: refreshes cfg.is_weapon / is_ak47 / etc. each frame
	void UpdateWeaponBooleans();
};
 