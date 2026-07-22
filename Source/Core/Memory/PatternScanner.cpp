#include "PatternScanner.hpp"
#include <Windows.h>
#include <Psapi.h>
#include <spdlog/spdlog.h>

std::vector<PatternScanner::PatternByte> PatternScanner::ParsePattern(const std::string& pattern)
{
	std::vector<PatternByte> bytes;
	bytes.reserve(pattern.size() / 3);

	for (size_t i = 0; i < pattern.size(); )
	{
		while (i < pattern.size() && pattern[i] == ' ') i++;

		if (i >= pattern.size()) break;

		if (pattern[i] == '?' || (i + 1 < pattern.size() && pattern[i + 1] == '?'))
		{
			bytes.push_back({ 0x00, true });
			i += (pattern[i] == '?') ? 1 : 2;
			if (i < pattern.size() && pattern[i] == '?') i++;
		}
		else if (i + 1 < pattern.size())
		{
			char hex[3] = { pattern[i], pattern[i + 1], '\0' };
			bytes.push_back({ static_cast<uint8_t>(strtoul(hex, nullptr, 16)), false });
			i += 2;
		}
		else
		{
			i++;
		}
	}

	return bytes;
}

std::optional<uintptr_t> PatternScanner::FindPattern(uintptr_t start, size_t size, const std::string& pattern)
{
	auto bytes = ParsePattern(pattern);
	if (bytes.empty()) return std::nullopt;

	const uint8_t* scanStart = reinterpret_cast<const uint8_t*>(start);
	const uint8_t* scanEnd = scanStart + size - bytes.size();

	for (const uint8_t* current = scanStart; current <= scanEnd; ++current)
	{
		bool found = true;
		for (size_t j = 0; j < bytes.size(); ++j)
		{
			if (!bytes[j].wildcard && current[j] != bytes[j].value)
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return reinterpret_cast<uintptr_t>(current);
		}
	}

	return std::nullopt;
}

std::optional<uintptr_t> PatternScanner::FindPatternInModule(const char* moduleName, const std::string& pattern)
{
	HMODULE hModule = GetModuleHandleA(moduleName);
	if (!hModule)
	{
		spdlog::error("PatternScanner: module \"{}\" not found", moduleName);
		return std::nullopt;
	}

	MODULEINFO moduleInfo{};
	if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo)))
	{
		spdlog::error("PatternScanner: failed to get module info for \"{}\"", moduleName);
		return std::nullopt;
	}

	uintptr_t base = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
	size_t size = moduleInfo.SizeOfImage;

	auto result = FindPattern(base, size, pattern);
	if (!result)
	{
		spdlog::error("PatternScanner: pattern not found in \"{}\"", moduleName);
	}
	return result;
}

std::vector<uintptr_t> PatternScanner::FindAllPatterns(uintptr_t start, size_t size, const std::string& pattern)
{
	std::vector<uintptr_t> results;
	auto bytes = ParsePattern(pattern);
	if (bytes.empty()) return results;

	const uint8_t* scanStart = reinterpret_cast<const uint8_t*>(start);
	const uint8_t* scanEnd = scanStart + size - bytes.size();

	for (const uint8_t* current = scanStart; current <= scanEnd; ++current)
	{
		bool found = true;
		for (size_t j = 0; j < bytes.size(); ++j)
		{
			if (!bytes[j].wildcard && current[j] != bytes[j].value)
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			results.push_back(reinterpret_cast<uintptr_t>(current));
		}
	}

	return results;
}

std::vector<uintptr_t> PatternScanner::FindAllPatternsInModule(const char* moduleName, const std::string& pattern)
{
	HMODULE hModule = GetModuleHandleA(moduleName);
	if (!hModule) return {};

	MODULEINFO moduleInfo{};
	if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo)))
		return {};

	return FindAllPatterns(
		reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll),
		moduleInfo.SizeOfImage,
		pattern
	);
}

uintptr_t PatternScanner::ResolveWithOffset(const char* moduleName, const std::string& pattern, ptrdiff_t offset)
{
	auto addr = FindPatternInModule(moduleName, pattern);
	if (!addr) return 0;
	return *addr + offset;
}
