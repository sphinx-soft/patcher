#include "scanner.hpp"
#include <sstream>
#include <iostream>
int total_pattern_count = 0;
int found_pattern_count = 0;

ptr_manage::ptr_manage(void* hand) { m_Ptr = hand; }
ptr_manage::ptr_manage(std::uintptr_t hand) { m_Ptr = reinterpret_cast<void*>(hand); }

ptr_manage ptr_manage::add(int offset)
{
	return ptr_manage(as<std::uintptr_t>() + offset);
}

find_pattern::find_pattern(const char* pattern)
{
	auto toUpper = [](char c) -> char
	{
		return c >= 'a' && c <= 'z' ? static_cast<char>(c + ('A' - 'a')) : static_cast<char>(c);
	};
	auto isHex = [&](char c) -> bool
	{
		switch (toUpper(c))
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			return true;
		default:
			return false;
		}
	};
	do
	{
		if (*pattern == ' ')
			continue;
		if (*pattern == '?')
		{
			Element e = Element({}, true);
			m_Elements.push_back(e);
			continue;
		}
		if (*(pattern + 1) && isHex(*pattern) && isHex(*(pattern + 1)))
		{
			char str[3] = { *pattern, *(pattern + 1), '\0' };
			auto data = std::strtol(str, nullptr, 16);

			Element e = Element(static_cast<std::uint8_t>(data), false);
			m_Elements.push_back(e);
		}
	} while (*(pattern++));
}
std::vector<ptr_manage> find_pattern::Scan(std::vector<unsigned char> buffer)
{
	std::vector<ptr_manage> result;
	auto compareMemory = [&](int index, Element* elem, std::size_t num) -> bool
	{
		for (int i = 0; i < num; ++i)
		{
			if (!elem[i].m_Wildcard) {
				if (i + index >= buffer.size()) 
					return false;

				if (buffer.at(i + index) != elem[i].m_Data)
					return false;
				
			}
		}
		return true;
	};
	for (int i = 0, end = buffer.size(); i != end; ++i)
	{
		if (compareMemory(i, m_Elements.data(), m_Elements.size()))
		{
			result.push_back(ptr_manage(i));
		}
	}
	return result;
}



pattern_hisnt::pattern_hisnt(std::string name, find_pattern pattern, std::function<void(ptr_manage)> callback, bool hook) :
	m_name(std::move(name)),
	m_pattern(std::move(pattern)),
	m_callback(std::move(callback)),
	m_hooked(hook)
{}

void pattern_batch::add(std::string name, find_pattern pattern, std::function<void(ptr_manage)> callback, bool hook) {
	m_patterns.emplace_back(name, pattern, callback, hook);
	total_pattern_count++;
}

bool pattern_batch::run(std::vector<unsigned char> buffer) {
	bool all_found = true;
	for (auto &entry : m_patterns)
	{
		auto result = entry.m_pattern.Scan(buffer);
		if (result.size() == 0) {
			all_found = false;
			continue;
		}
		if (entry.m_callback)
		{
			for (int i = 0; i < result.size(); i++) {
				if (result[i].as<uintptr_t>() != NULL) {
					found_pattern_count++;
					std::invoke(std::move(entry.m_callback), result[i]);
				}
			}
			
		}
	}

	m_patterns.clear();
	return all_found;
}