#pragma once
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <array>
#include <fstream>
#include <string>

struct KeyBind
{
	int key = 0;
	int mode = 0; // 0: Hold, 1: Toggle, 2: Always On
	bool active = false;

	bool UpdateState()
	{
		if (key == 0) return true;
		bool pressed = (GetAsyncKeyState(key) & 0x8000) != 0;

		if (mode == 2) // Always On
		{
			active = true;
		}
		else if (mode == 0) // Hold
		{
			active = pressed;
		}
		else if (mode == 1) // Toggle
		{
			static bool wasPressed = false;
			if (pressed && !wasPressed)
				active = !active;
			wasPressed = pressed;
		}
		return active;
	}
};

// Aimbot target selection modes
enum struct EAimbotMode : int
{
	Closest   = 0,  // nearest by world distance
	LowestHP  = 1,  // target with least HP
	HighestHP = 2,  // target with most HP (e.g. tank priority)
	MostKills = 3,  // most kills / most dangerous
};

struct Config
{
	bool bRenderFinished = false;
	bool bEspOnTeammates = false;
	bool bZoomOverride = false;
	bool bEspOnEnemies = false;
	bool bUnloadCheat = false;
	bool bHealthBar = false;
	bool bArmorBar = false;
	bool bBoxes = false;
	bool bBhop = false;
	bool bFly = false;
	bool bEsp = false;
	bool bArrows = false;
	bool bCircle = false;
	bool bSnaplines = false;
	bool bLines = false;
	bool bNames = false;

	bool bAimbot = false;
	bool bAimFovCircle = true;
	bool bSilentAim = false;
	float fAimFov = 90.0f;
	int iAimKey = 0x02;
	// Mode: 0=Closest, 1=LowestHP, 2=HighestHP, 3=MostKills
	int iAimbotMode = 0;
	bool bAimPrediction = true;
	float fAimPredictionFactor = 1.0f;
	float fAimBulletSpeed = 800.0f;
	// Filter targets by weapon boolean flags
	bool bAimOnlyWhenWeapon = false;  // skip unarmed targets
	bool bAimIgnoreSniper = false;    // ignore targets with sniper/bow

	bool bRapidFire = false;
	// Multiplier applied to fireRateCurrent (lower = faster; 0 = instant)
	float fRapidFireMultiplier = 0.0f;

	bool bChams = false;
	bool bChamsEnemyOnly = true;    // don't draw chams on teammates
	bool bChamsThroughWalls = true; // show even when occluded
	std::array<float, 4> colChamsEnemy    = { 1.0f, 0.1f, 0.1f, 0.85f };
	std::array<float, 4> colChamsFriendly = { 0.1f, 0.6f, 1.0f, 0.85f };
	std::array<float, 4> colChamsLocal    = { 0.1f, 1.0f, 0.3f, 0.85f };

	// Weapon boolean flags (runtime state, not serialized)
	// Set each frame by the cheat loop based on local player's active weapon.
	bool is_weapon       = false;  // local player is holding any gun
	bool is_ak47         = false;
	bool is_m16          = false;
	bool is_smg          = false;
	bool is_shotgun      = false;
	bool is_sniper       = false;
	bool is_heavy        = false;  // RPG / Minigun
	bool is_melee        = false;

	bool bVehicleFly = false;
	float fVehicleFlySpeed = 25.0f;
	bool bAntiBanana = true;
	bool bInfiniteRoll = true;
	bool bVehicleBoost = true;

	bool bFeatureList = true;
	bool bVelocityIndicator = false;

	KeyBind kbEsp;
	KeyBind kbBoxes;
	KeyBind kbSnaplines;
	KeyBind kbArrows;
	KeyBind kbNames;
	KeyBind kbArmorEsp;
	KeyBind kbAim;
	KeyBind kbGrenadeEsp;
	KeyBind kbBhop;
	KeyBind kbFastParachute;
	KeyBind kbDisableMoveAnim;
	KeyBind kbVehicleFly;
	KeyBind kbFeatureList;
	KeyBind kbVelocityIndicator;
	KeyBind kbRapidFire;
	KeyBind kbChams;
	KeyBind kbSkeleton;
	KeyBind kbAntiBanana;
	KeyBind kbInfiniteRoll;

	float fZoomDefaultValue = 67.5f;
	float fZoomOverrideValue = fZoomDefaultValue;
	float fArrowsRadius = 80.0f;
	float fArrowThickness = 2.0f;
	float fSnaplineLength = 400.0f;
	float fNearSnaplineRenderDistance = 200.0f;
	float fMaxSnaplineRenderDistance = 500.0f;
	float fFlySpeed = 50.0f;
	float fBhopDelay = 0.0f;
	bool bSkeletonEsp = true;
	bool bArmorEsp = true;
	bool bGrenadeEsp = true;
	bool bBhopRollMode = false;
	bool bDisableMoveAnim = false;
	bool bFastParachute = false;
	float fParachuteDropSpeed = 10.0f;
	float fSpeedbunny = 1.0f;
	std::array<float, 4> colBox       = { 1.0f, 0.0f, 0.0f, 1.0f };
	std::array<float, 4> colName      = { 1.0f, 1.0f, 1.0f, 1.0f };
	std::array<float, 4> colArmor     = { 0.2f, 0.6f, 1.0f, 1.0f };
	std::array<float, 4> colGrenade   = { 1.0f, 0.8f, 0.2f, 1.0f };
	std::array<float, 4> colSkeleton  = { 0.0f, 1.0f, 0.5f, 1.0f };
	std::array<float, 4> colFovCircle = { 1.0f, 1.0f, 1.0f, 0.4f };

	std::vector<int16_t> friendIDs;

	bool IsFriend(int16_t id) const
	{
		for (auto f : friendIDs)
			if (f == id) return true;
		return false;
	}

	void ToggleFriend(int16_t id)
	{
		for (auto it = friendIDs.begin(); it != friendIDs.end(); ++it)
		{
			if (*it == id)
			{
				friendIDs.erase(it);
				return;
			}
		}
		friendIDs.push_back(id);
	}

	bool SaveToFile(const std::string& path = "default_config.json") const;
	bool LoadFromFile(const std::string& path = "default_config.json");
	void Reset();
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	Config,
	fArrowsRadius,
	fArrowThickness,
	fSnaplineLength,
	fNearSnaplineRenderDistance,
	fMaxSnaplineRenderDistance,
	fZoomOverrideValue,
	bEspOnTeammates,
	bEspOnEnemies,
	bZoomOverride,
	bHealthBar,
	bArmorBar,
	bBoxes,
	bBhop,
	bEsp,
	bArrows,
	bCircle,
	bSnaplines,
	bLines,
	bNames,
	bAimbot,
	bAimFovCircle,
	fAimFov,
	iAimKey,
	iAimbotMode,
	bAimPrediction,
	fAimPredictionFactor,
	fAimBulletSpeed,
	bAimOnlyWhenWeapon,
	bAimIgnoreSniper,
	bRapidFire,
	fRapidFireMultiplier,
	bChams,
	bChamsEnemyOnly,
	bChamsThroughWalls,
	bFly,
	fFlySpeed,
	fBhopDelay,
	bArmorEsp,
	bGrenadeEsp,
	bBhopRollMode,
	bDisableMoveAnim,
	bFastParachute,
	fParachuteDropSpeed,
	fSpeedbunny,
	bVehicleFly,
	fVehicleFlySpeed,
	bFeatureList,
	bVelocityIndicator
)

inline bool Config::SaveToFile(const std::string& path) const
{
	try
	{
		nlohmann::json j = *this;
		j["colBox"]          = colBox;
		j["colName"]         = colName;
		j["colArmor"]        = colArmor;
		j["colGrenade"]      = colGrenade;
		j["colChamsEnemy"]   = colChamsEnemy;
		j["colChamsFriendly"]= colChamsFriendly;
		j["colChamsLocal"]   = colChamsLocal;
		j["friendIDs"]       = friendIDs;

		auto saveKeyBind = [&](const std::string& name, const KeyBind& kb)
		{
			j[name] = { {"key", kb.key}, {"mode", kb.mode} };
		};

		saveKeyBind("kbEsp", kbEsp);
		saveKeyBind("kbBoxes", kbBoxes);
		saveKeyBind("kbSnaplines", kbSnaplines);
		saveKeyBind("kbArrows", kbArrows);
		saveKeyBind("kbNames", kbNames);
		saveKeyBind("kbArmorEsp", kbArmorEsp);
		saveKeyBind("kbAim", kbAim);
		saveKeyBind("kbGrenadeEsp", kbGrenadeEsp);
		saveKeyBind("kbBhop", kbBhop);
		saveKeyBind("kbFastParachute", kbFastParachute);
		saveKeyBind("kbDisableMoveAnim", kbDisableMoveAnim);
		saveKeyBind("kbVehicleFly", kbVehicleFly);
		saveKeyBind("kbFeatureList", kbFeatureList);
		saveKeyBind("kbVelocityIndicator", kbVelocityIndicator);
		saveKeyBind("kbRapidFire", kbRapidFire);
		saveKeyBind("kbChams", kbChams);

		std::ofstream file(path);
		if (!file.is_open()) return false;
		file << j.dump(4);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

inline bool Config::LoadFromFile(const std::string& path)
{
	try
	{
		std::ifstream file(path);
		if (!file.is_open()) return false;

		nlohmann::json j;
		file >> j;
		j.get_to(*this);

		if (j.contains("colBox"))           colBox           = j["colBox"].get<std::array<float,4>>();
		if (j.contains("colName"))          colName          = j["colName"].get<std::array<float,4>>();
		if (j.contains("colArmor"))         colArmor         = j["colArmor"].get<std::array<float,4>>();
		if (j.contains("colGrenade"))       colGrenade       = j["colGrenade"].get<std::array<float,4>>();
		if (j.contains("colChamsEnemy"))    colChamsEnemy    = j["colChamsEnemy"].get<std::array<float,4>>();
		if (j.contains("colChamsFriendly")) colChamsFriendly = j["colChamsFriendly"].get<std::array<float,4>>();
		if (j.contains("colChamsLocal"))    colChamsLocal    = j["colChamsLocal"].get<std::array<float,4>>();
		if (j.contains("friendIDs"))        friendIDs        = j["friendIDs"].get<std::vector<int16_t>>();

		auto loadKeyBind = [&](const std::string& name, KeyBind& kb)
		{
			if (j.contains(name))
			{
				kb.key  = j[name].value("key",  0);
				kb.mode = j[name].value("mode", 0);
			}
		};

		loadKeyBind("kbEsp",             kbEsp);
		loadKeyBind("kbBoxes",           kbBoxes);
		loadKeyBind("kbSnaplines",       kbSnaplines);
		loadKeyBind("kbArrows",          kbArrows);
		loadKeyBind("kbNames",           kbNames);
		loadKeyBind("kbArmorEsp",        kbArmorEsp);
		loadKeyBind("kbAim",             kbAim);
		loadKeyBind("kbGrenadeEsp",      kbGrenadeEsp);
		loadKeyBind("kbBhop",            kbBhop);
		loadKeyBind("kbFastParachute",   kbFastParachute);
		loadKeyBind("kbDisableMoveAnim", kbDisableMoveAnim);
		loadKeyBind("kbVehicleFly",      kbVehicleFly);
		loadKeyBind("kbFeatureList",     kbFeatureList);
		loadKeyBind("kbVelocityIndicator", kbVelocityIndicator);
		loadKeyBind("kbRapidFire",       kbRapidFire);
		loadKeyBind("kbChams",           kbChams);

		return true;
	}
	catch (...)
	{
		return false;
	}
}

inline void Config::Reset()
{
	*this = Config();
}
