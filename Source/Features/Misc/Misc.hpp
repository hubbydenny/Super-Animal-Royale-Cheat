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
};
