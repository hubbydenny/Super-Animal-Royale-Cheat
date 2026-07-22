#pragma once
#include "../../Core/Modules/ModuleBase.hpp"
constexpr const char* AIMBOT_MODULE_NAME = "Aimbot";

class AimbotModule final : public ModuleBase
{
public:
	explicit AimbotModule(ModulesManager* const modules, HooksManager* const hooks, Config& cfg);
	void Run() override;
};
