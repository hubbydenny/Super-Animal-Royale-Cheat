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
	float fAimFov = 90.0f;
	int iAimKey = 0x02;
	int iAimMode = 0;

	bool bVehicleFly = false;
	float fVehicleFlySpeed = 25.0f;

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
    float fZoomDefaultValue = 67.5f;
	float fZoomOverrideValue = fZoomDefaultValue;
	float fArrowsRadius = 80.0f;
	float fArrowThickness = 2.0f;
	float fSnaplineLength = 400.0f;
	float fNearSnaplineRenderDistance = 200.0f;
	float fMaxSnaplineRenderDistance = 500.0f;
	float fFlySpeed = 50.0f;
	float fBhopDelay = 0.0f;
	bool bArmorEsp = true;
	bool bGrenadeEsp = true;
	bool bBhopRollMode = false;
	bool bDisableMoveAnim = false;
	bool bFastParachute = false;
	float fParachuteDropSpeed = 10.0f;
	float fSpeedbunny = 1.0f;
	std::array<float, 4> colBox = { 1.0f, 0.0f, 0.0f, 1.0f };
	std::array<float, 4> colName = { 1.0f, 1.0f, 1.0f, 1.0f };
	std::array<float, 4> colArmor = { 0.2f, 0.6f, 1.0f, 1.0f };
	std::array<float, 4> colGrenade = { 1.0f, 0.8f, 0.2f, 1.0f };

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
	iAimMode,
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
		j["colBox"] = colBox;
		j["colName"] = colName;
		j["colArmor"] = colArmor;
		j["colGrenade"] = colGrenade;
		j["friendIDs"] = friendIDs;

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

		if (j.contains("colBox")) colBox = j["colBox"].get<std::array<float, 4>>();
		if (j.contains("colName")) colName = j["colName"].get<std::array<float, 4>>();
		if (j.contains("colArmor")) colArmor = j["colArmor"].get<std::array<float, 4>>();
		if (j.contains("colGrenade")) colGrenade = j["colGrenade"].get<std::array<float, 4>>();
		if (j.contains("friendIDs")) friendIDs = j["friendIDs"].get<std::vector<int16_t>>();

		auto loadKeyBind = [&](const std::string& name, KeyBind& kb)
		{
			if (j.contains(name))
			{
				kb.key = j[name].value("key", 0);
				kb.mode = j[name].value("mode", 0);
			}
		};

		loadKeyBind("kbEsp", kbEsp);
		loadKeyBind("kbBoxes", kbBoxes);
		loadKeyBind("kbSnaplines", kbSnaplines);
		loadKeyBind("kbArrows", kbArrows);
		loadKeyBind("kbNames", kbNames);
		loadKeyBind("kbArmorEsp", kbArmorEsp);
		loadKeyBind("kbAim", kbAim);
		loadKeyBind("kbGrenadeEsp", kbGrenadeEsp);
		loadKeyBind("kbBhop", kbBhop);
		loadKeyBind("kbFastParachute", kbFastParachute);
		loadKeyBind("kbDisableMoveAnim", kbDisableMoveAnim);
		loadKeyBind("kbVehicleFly", kbVehicleFly);
		loadKeyBind("kbFeatureList", kbFeatureList);
		loadKeyBind("kbVelocityIndicator", kbVelocityIndicator);

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
