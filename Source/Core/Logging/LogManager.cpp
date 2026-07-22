#include "LogManager.hpp"
#include <iostream>
#include <Windows.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

void LogManager::Init() noexcept
{
    try
    {
        auto file_logger = spdlog::basic_logger_mt("hack", "SuperAnimalHack.log", true);
        spdlog::set_default_logger(file_logger);
        spdlog::set_level(spdlog::level::debug);
    }
    catch (...)
    {
        spdlog::set_level(spdlog::level::off);
    }
}

void LogManager::Shutdown() noexcept
{
	spdlog::shutdown();
}
