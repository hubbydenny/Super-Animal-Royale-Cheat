#pragma once
#include "../../Core/Modules/ModuleBase.hpp"
#include "../../Core/Math/Vectors.hpp"
#include <unordered_map>

constexpr const char* ESP_MODULE_NAME = "ESP";

class EspModule final : public ModuleBase
{
public:
	explicit EspModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg);

	void Run();
	void DrawArrows();
	void DrawBoxes();
	void DrawNames();
	void DrawSnaplines();
	void DrawArmor();
	void DrawGrenades();
	void DrawSkeleton();

	void DrawChams();

private:
	struct SmoothedPlayerPos {
		Vector2 currentPos{0, 0};
		ULONGLONG lastUpdateMs{0};
	};
	std::unordered_map<int16_t, SmoothedPlayerPos> m_smoothedPosMap;
	Vector2 GetSmoothedPos(int16_t playerID, const Vector2& rawPos);
};
