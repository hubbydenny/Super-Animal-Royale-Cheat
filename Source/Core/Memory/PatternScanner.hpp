#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

namespace PatternScanner
{
	struct PatternByte
	{
		uint8_t value;
		bool wildcard;
	};

	std::vector<PatternByte> ParsePattern(const std::string& pattern);

	std::optional<uintptr_t> FindPattern(uintptr_t start, size_t size, const std::string& pattern);
	std::optional<uintptr_t> FindPatternInModule(const char* moduleName, const std::string& pattern);

	std::vector<uintptr_t> FindAllPatterns(uintptr_t start, size_t size, const std::string& pattern);
	std::vector<uintptr_t> FindAllPatternsInModule(const char* moduleName, const std::string& pattern);

	uintptr_t ResolveWithOffset(const char* moduleName, const std::string& pattern, ptrdiff_t offset);
}
