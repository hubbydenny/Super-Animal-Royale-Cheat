#include "ModulesManager.hpp"

ModulesManager::ModulesManager(HooksManager& hooks)
	: m_hooks(hooks), m_cfg()
{}

void ModulesManager::Init()
{
	m_cfg.LoadFromFile("default_config.json");

	for (const auto& module : m_modules)
	{
		module->Init();
	}
}

void ModulesManager::Run()
{
	m_cfg.kbEsp.UpdateState();
	m_cfg.kbBoxes.UpdateState();
	m_cfg.kbSnaplines.UpdateState();
	m_cfg.kbArrows.UpdateState();
	m_cfg.kbNames.UpdateState();
	m_cfg.kbArmorEsp.UpdateState();
	m_cfg.kbAim.UpdateState();
	m_cfg.kbGrenadeEsp.UpdateState();
	m_cfg.kbBhop.UpdateState();
	m_cfg.kbFastParachute.UpdateState();
	m_cfg.kbDisableMoveAnim.UpdateState();
	m_cfg.kbVehicleFly.UpdateState();
	m_cfg.kbFeatureList.UpdateState();
	m_cfg.kbVelocityIndicator.UpdateState();

	for (const auto& module : m_modules)
	{
		module->Run();
	}
}

void ModulesManager::Shutdown()
{
	m_cfg.SaveToFile("default_config.json");

	for (const auto& module : m_modules)
	{
		module->Shutdown();
	}
}

Config& ModulesManager::GetConfig() noexcept
{
	return m_cfg;
}
