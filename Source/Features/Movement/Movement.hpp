#pragma once
#include "../../Core/Modules/ModuleBase.hpp"

constexpr const char* MOVEMENT_MODULE_NAME = "MOVEMENT";

class MovementModule final : public ModuleBase
{
public:
	explicit MovementModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg);

	void Run();
};
