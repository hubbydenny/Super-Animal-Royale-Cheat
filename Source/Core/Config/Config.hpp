#pragma once
#include <nlohmann/json.hpp>

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
	bool bEsp = false;
	bool bArrows = false;
	bool bCircle = false;
	bool bSnaplines = false;
	bool bLines = false;
	bool bNames = false;
    const float fZoomDefaultValue = 67.5f;
	float fZoomOverrideValue = fZoomDefaultValue;
	float fArrowsRadius = 80.0f;
	float fArrowThickness = 2.0f;
	float fSnaplineLength = 400.0f;
	float fNearSnaplineRenderDistance = 200.0f;
	float fMaxSnaplineRenderDistance = 500.0f;


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
		bNames
	);
};
