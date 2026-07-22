#pragma once
#include <cstdint>

class NopBuffer final
{
public:
	explicit NopBuffer(uintptr_t address, size_t length);
	NopBuffer() : m_address(0), m_length(0), m_bytes(nullptr), m_bNopped(false) {}
	NopBuffer& operator=(const NopBuffer&) = delete;
	NopBuffer& operator=(NopBuffer&& other) noexcept;
	~NopBuffer();

	void Nop();
	void Restore();

private:
	unsigned char* m_bytes;
	bool m_bNopped;

	uintptr_t m_address;
	size_t m_length;
};