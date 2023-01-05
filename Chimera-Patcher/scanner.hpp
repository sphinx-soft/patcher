#pragma once
#include <string>
#include <vector>
#include <functional>

class ptr_manage {
public:
	ptr_manage(void* hand = 0);
	ptr_manage(std::uintptr_t hand = 0);

	template <typename T>
	inline std::enable_if_t<std::is_pointer<T>::value, T> as()
	{
		return static_cast<T>(m_Ptr);
	}

	template <typename T>
	inline std::enable_if_t<std::is_lvalue_reference<T>::value, T> as()
	{
		return *static_cast<std::add_pointer_t<std::remove_reference_t<T>>>(m_Ptr);
	}

	template <typename T>
	inline std::enable_if_t<std::is_same<T, std::uintptr_t>::value, T> as()
	{
		return reinterpret_cast<T>(m_Ptr);
	}

	ptr_manage add(int offset);

	inline operator bool() { return m_Ptr != nullptr; }

private:
	void* m_Ptr;
};

class find_pattern {
public:
	struct Element
	{
		std::uint8_t m_Data{};
		bool m_Wildcard{};
		Element(uint8_t data, bool wildcard) :
			m_Data(data), m_Wildcard(wildcard)
		{ }
	};

	find_pattern(const char* pattern);

	std::vector<ptr_manage> Scan(std::vector<unsigned char> buffer);

private:
	const char* m_pat;
	std::vector<Element> m_Elements;
};

struct pattern_hisnt
{
	std::string m_name;
	find_pattern m_pattern;
	std::function<void(ptr_manage)> m_callback;
	bool m_hooked;

	pattern_hisnt(std::string name, find_pattern pattern, std::function<void(ptr_manage)> callback, bool hook = false);
};

class pattern_batch {
public:
	void add(std::string name, find_pattern pattern, std::function<void(ptr_manage)> callback, bool hook = false);
	bool run(std::vector<unsigned char> buffer);
private:
	std::vector<pattern_hisnt> m_patterns;
};
