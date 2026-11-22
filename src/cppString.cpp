// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "cppString.h"
#include "utf8.h"
#include <Windows.h>

cppstr::String_Error::String_Error(const String& msg)             : message(msg) {}
cppstr::String_Error::String_Error(const char* msg)                    : message(msg ? msg : "The string has errors.") {}
[[nodiscard]] const char* cppstr::String_Error::what()                 const noexcept { return message.c_str(); }

cppstr::Index_Out_Of_Range::Index_Out_Of_Range(const String& msg) : String_Error(msg) {}
cppstr::Index_Out_Of_Range::Index_Out_Of_Range(const char* msg)        : String_Error(msg ? msg : "Index out of string range.") {}

cppstr::Empty_String::Empty_String(const String& msg)             : String_Error(msg) {}
cppstr::Empty_String::Empty_String(const char* msg)                    : String_Error(msg ? msg : "Empty string.") {}

cppstr::Invalid_Operation::Invalid_Operation(const String& msg)   : String_Error(msg) {}
cppstr::Invalid_Operation::Invalid_Operation(const char* msg)          : String_Error(msg ? msg : "Invalid Operation.") {}

cppstr::Invalid_Conversion::Invalid_Conversion(const String& msg)      : String_Error(msg) {}
cppstr::Invalid_Conversion::Invalid_Conversion(const char* msg)        : String_Error(msg ? msg : "Invalid Conversion.") {}

cppstr::Invalid_Argument::Invalid_Argument(const String& msg)          : String_Error(msg) {}
cppstr::Invalid_Argument::Invalid_Argument(const char* msg)            : String_Error(msg ? msg : "Invalid Argument.") {}

cppstr::cppString::cppString() {}
cppstr::cppString::cppString(const String& _data)           : m_data(_data) {}
cppstr::cppString::cppString(String&& _data)                : m_data(std::move(_data)) {}
cppstr::cppString::cppString(const char* _data)                  : m_data(_data) {}
cppstr::cppString::cppString(std::initializer_list<char> _data)
{
    for (auto& c : _data)
    {
        m_data += c;
    }
}
cppstr::cppString::cppString(std::vector<char> _data)
{
    for (auto& c : _data)
    {
        m_data += c;
    }
}
cppstr::cppString::cppString(std::set<char> _data)
{
	for (auto& c : _data)
	{
		m_data += c;
	}
}
cppstr::cppString::cppString(const cppString& _data)           : m_data(_data.m_data) {}
cppstr::cppString::cppString(String _data, size_type size) : m_data(_data.substr(0, size)) {}

cppstr::cppString cppstr::cppString::create_from_this()                       const { return cppstr::cppString(m_data); }

cppstr::cppString& cppstr::cppString::operator=(const String& _data)     { m_data = _data; return *this; }
cppstr::cppString& cppstr::cppString::operator=(String&& _data) noexcept { m_data = std::move(_data); return *this; }
cppstr::cppString& cppstr::cppString::operator=(const char* _data)            { m_data = (_data ? _data : ""); return *this; }

const String& cppstr::cppString::s_str() const& noexcept { return m_data; }
String&& cppstr::cppString::s_str() &&   noexcept { return std::move(m_data); }
const char* cppstr::cppString::c_str()   const noexcept { return m_data.c_str(); }
const char* cppstr::cppString::data()    const noexcept { return m_data.data(); }
char* cppstr::cppString::data()          noexcept { return &m_data[0]; }
String cppstr::cppString::operator*()    const { return m_data; }
String& cppstr::cppString::get_changable_string()   { return m_data; }

cppstr::cppString::operator std::string()     const& { return m_data; }
cppstr::cppString::operator std::string() &&  { return std::move(m_data); }
cppstr::cppString::operator const char* ()    const { return c_str(); }
cppstr::cppString::operator bool()            const { return !empty(); }

bool cppstr::cppString::empty()                          const noexcept { return m_data.empty(); }
cppstr::cppString::size_type cppstr::cppString::size()      const noexcept { return m_data.size(); }
cppstr::cppString::size_type cppstr::cppString::length()    const noexcept { return m_data.length(); }
cppstr::cppString::size_type cppstr::cppString::capacity()  const noexcept { return m_data.capacity(); }
void cppstr::cppString::reserve(size_type new_cap)       { m_data.reserve(new_cap); }
void cppstr::cppString::shrink_to_fit()                  { m_data.shrink_to_fit(); }

void cppstr::cppString::clear()                                                            noexcept { m_data.clear(); }

cppstr::cppString& cppstr::cppString::insert(pos_type pos, const cppString& str)
{
	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in insert.");

	m_data.insert(pos, str.m_data);
	return *this;
}

cppstr::cppString& cppstr::cppString::insert(pos_type pos, const String& str)
{
	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in insert.");

	m_data.insert(pos, str);
	return *this;
}

cppstr::cppString& cppstr::cppString::insert(pos_type pos, const char* str)
{
	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in insert.");

	m_data.insert(pos, str ? str : "");
	return *this;
}

cppstr::cppString& cppstr::cppString::insert(pos_type pos, char ch)
{
	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in insert.");

	m_data.insert(pos, 1, ch);
	return *this;
}

cppstr::cppString& cppstr::cppString::erase(pos_type pos, pos_type count)
{
	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in erase.");

	m_data.erase(pos, count);
	return *this;
}

cppstr::cppString& cppstr::cppString::assign(const cppString& str)
{
	m_data = str.m_data;
	return *this;
}

cppstr::cppString& cppstr::cppString::assign(const String& str)
{
	m_data = str;
	return *this;
}

cppstr::cppString& cppstr::cppString::assign(const char* str)
{
	m_data = (str ? str : "");
	return *this;
}

cppstr::cppString& cppstr::cppString::assign(pos_type count, char ch)
{
	m_data.assign(count, ch);
	return *this;
}

void cppstr::cppString::resize(size_type count)
{
	m_data.resize(count);
}

void cppstr::cppString::resize(size_type count, char fill)
{
	m_data.resize(count, fill);
}

int cppstr::cppString::compare(const cppString& rhs) const
{
	return m_data.compare(rhs.m_data);
}

int cppstr::cppString::compare(const String& rhs) const
{
	return m_data.compare(rhs);
}

int cppstr::cppString::compare(const char* rhs) const
{
	return m_data.compare(rhs ? rhs : "");
}

cppstr::cppString::pos_type cppstr::cppString::rfind(const cppString& s, pos_type pos) const noexcept
{
	return m_data.rfind(s.m_data, pos);
}

cppstr::cppString::pos_type cppstr::cppString::rfind(const char* s, pos_type pos) const noexcept
{
	return m_data.rfind(s ? s : "", pos);
}

cppstr::cppString::pos_type cppstr::cppString::rfind(char ch, pos_type pos) const noexcept
{
	return m_data.rfind(ch, pos);
}

cppstr::cppString& cppstr::cppString::append(const String& s)                            { m_data.append(s); return *this; }
cppstr::cppString& cppstr::cppString::append(const char* s, cppstr::cppString::size_type count)  { m_data.append(s, count); return *this; }
cppstr::cppString& cppstr::cppString::append(const char* s)                                   { m_data.append(s); return *this; }
cppstr::cppString& cppstr::cppString::append(cppstr::cppString::size_type count, char ch)        { m_data.append(count, ch); return *this; }

cppstr::cppString& cppstr::cppString::operator+=(const cppString& rhs)   { m_data += rhs.m_data; return *this; }
cppstr::cppString& cppstr::cppString::operator+=(const String& rhs)  { m_data += rhs; return *this; }
cppstr::cppString& cppstr::cppString::operator+=(const char* rhs)         { m_data += rhs; return *this; }
cppstr::cppString& cppstr::cppString::operator+=(char ch)                 { m_data += ch; return *this; }

cppstr::cppString cppstr::cppString::operator+(const cppString& rhs) const { return cppString(m_data + rhs.m_data); }
cppstr::cppString cppstr::cppString::operator+(const String& rhs) const { return cppString(m_data + rhs); }
cppstr::cppString cppstr::cppString::operator+(const char* rhs) const { return cppString(m_data + (rhs ? rhs : "")); }

cppstr::cppString cppstr::operator+(const char* lhs, const cppstr::cppString& rhs) { return cppstr::cppString((lhs ? lhs : "") + rhs.s_str()); }
cppstr::cppString cppstr::operator+(const String& lhs, const cppstr::cppString& rhs) { return cppstr::cppString(lhs + rhs.s_str()); }

cppstr::cppString cppstr::cppString::operator*(cppstr::cppString::size_type n) const
{
    String _Data{};

	for (size_type i = 0; i < n; ++i)
	{
		_Data += m_data;
	}

    return cppString(_Data);
}

cppstr::cppString& cppstr::cppString::operator*=(cppstr::cppString::size_type n)
{
	if (n == 0)
	{
		m_data.clear();
		return *this;
	}

	const String base = m_data;
	for (size_type i = 1; i < n; ++i)
	{
		m_data += base;
	}
	return *this;
}

cppstr::cppString cppstr::cppString::operator-(size_type n) const
{
	cppString result = substr(n);

	return result;
}

cppstr::cppString& cppstr::cppString::operator-=(size_type n)
{
	if (n > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in operator-=.");

	if (n == m_data.size())
	{
		m_data.clear();
		return *this;
	}

	if (n == 0)
		return *this;

	if (n < 0)
		throw cppstr::Invalid_Argument("Negative value is not allowed in operator-=.");

	if (m_data.empty())
		throw cppstr::Empty_String("Cannot perform operator-= on an empty string.");

	m_data.resize(m_data.size() - n);

	return *this;
}

cppstr::cppString& cppstr::cppString::operator--()
{
	if (m_data.empty())
		throw cppstr::Empty_String("Cannot perform operator-- on an empty string.");

	if (m_data.size() == 1)
	{
		m_data.clear();
		return *this;
	}

	m_data.resize(m_data.size() - 1);

	return *this;
}

cppstr::cppString cppstr::cppString::operator&(const cppstr::cppString& rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs.m_data.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] &= rhs.m_data[i];
    }
    return result;
}
cppstr::cppString cppstr::cppString::operator&(const String& rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] &= rhs[i];
    }
    return result;
}
cppstr::cppString cppstr::cppString::operator&(const char* rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type rhs_len = std::strlen(rhs);
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs_len);
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] &= rhs[i];
    }
    return result;
}

cppstr::cppString cppstr::cppString::operator|(const cppstr::cppString& rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs.m_data.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] |= rhs.m_data[i];
    }
    return result;
}

cppstr::cppString cppstr::cppString::operator|(const String& rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] |= rhs[i];
    }
    return result;
}

cppstr::cppString cppstr::cppString::operator|(const char* rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type rhs_len = std::strlen(rhs);
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs_len);
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] |= rhs[i];
    }
    return result;
}


cppstr::cppString cppstr::cppString::operator^(const cppstr::cppString& rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs.m_data.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] ^= rhs.m_data[i];
    }
    return result;
}

cppstr::cppString cppstr::cppString::operator^(const String& rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] ^= rhs[i];
    }
    return result;
}

cppstr::cppString cppstr::cppString::operator^(const char* rhs) const
{
    cppstr::cppString result{ *this };
    cppstr::cppString::size_type rhs_len = std::strlen(rhs);
    cppstr::cppString::size_type min_size = (std::min)(result.m_data.size(), rhs_len);
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        result.m_data[i] ^= rhs[i];
    }
    return result;
}

cppstr::cppString& cppstr::cppString::operator&=(const cppstr::cppString& rhs)
{
    cppstr::cppString::size_type min_size = (std::min)(this->m_data.size(), rhs.m_data.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        m_data[i] &= rhs.m_data[i];
    }
    return *this;
}

cppstr::cppString& cppstr::cppString::operator|=(const cppstr::cppString& rhs)
{
    cppstr::cppString::size_type min_size = (std::min)(this->m_data.size(), rhs.m_data.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        m_data[i] |= rhs.m_data[i];
    }
    return *this;
}

cppstr::cppString& cppstr::cppString::operator^=(const cppstr::cppString& rhs)
{
    cppstr::cppString::size_type min_size = (std::min)(this->m_data.size(), rhs.m_data.size());
    for (cppstr::cppString::size_type i = 0; i < min_size; ++i) {
        m_data[i] ^= rhs.m_data[i];
    }
    return *this;
}

 char& cppstr::cppString::operator[](cppstr::cppString::size_type pos) 
{
	if (pos >= m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in operator[].");

    return m_data[pos]; 
}
 const char& cppstr::cppString::operator[](cppstr::cppString::size_type pos) const 
{
	if (pos >= m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in operator[].");

    return m_data[pos]; 
}

char& cppstr::cppString::at(cppstr::cppString::size_type pos)
{
    if (pos >= m_data.size())
        throw cppstr::Index_Out_Of_Range("Index out of range in cppString::at.");
    return m_data.at(pos);
}
const char& cppstr::cppString::at(cppstr::cppString::size_type pos) const
{
    if (pos >= m_data.size())
        throw cppstr::Index_Out_Of_Range("Index out of range in cppString::at.");
    return m_data.at(pos);
}

char& cppstr::cppString::front() { return m_data.front(); }
const char& cppstr::cppString::front() const { return m_data.front(); }
char& cppstr::cppString::back() { return m_data.back(); }
const char& cppstr::cppString::back() const { return m_data.back(); }

cppstr::cppString::iterator cppstr::cppString::begin()                   noexcept { return m_data.begin(); }
cppstr::cppString::const_iterator cppstr::cppString::begin()             const noexcept { return m_data.begin(); }
cppstr::cppString::const_iterator cppstr::cppString::cbegin()            const noexcept { return m_data.cbegin(); }
cppstr::cppString::iterator cppstr::cppString::end()                     noexcept { return m_data.end(); }
cppstr::cppString::const_iterator cppstr::cppString::end()               const noexcept { return m_data.end(); }
cppstr::cppString::const_iterator cppstr::cppString::cend()              const noexcept { return m_data.cend(); }
cppstr::cppString::reverse_iterator cppstr::cppString::rbegin()          noexcept { return m_data.rbegin(); }
cppstr::cppString::const_reverse_iterator cppstr::cppString::rbegin()    const noexcept { return m_data.rbegin(); }
cppstr::cppString::const_reverse_iterator cppstr::cppString::crbegin()   const noexcept { return m_data.crbegin(); }
cppstr::cppString::reverse_iterator cppstr::cppString::rend()            noexcept { return m_data.rend(); }
cppstr::cppString::const_reverse_iterator cppstr::cppString::rend()      const noexcept { return m_data.rend(); }
cppstr::cppString::const_reverse_iterator cppstr::cppString::crend()     const noexcept { return m_data.crend(); }


cppstr::cppString cppstr::cppString::substr(
    cppstr::cppString::pos_type pos,
    cppstr::cppString::pos_type count
) const
{
    if (pos > m_data.size())
        throw cppstr::Index_Out_Of_Range("Start index out of range in cppString::substr.");
    return cppstr::cppString(m_data.substr(pos, count));
}

cppstr::cppString cppstr::cppString::substr_between(
    cppstr::cppString::pos_type start_pos,
    cppstr::cppString::pos_type end_pos,
    cppstr::cppString::pos_type count
) const 
{
    using pos_t = cppstr::cppString::pos_type;
    const pos_t data_len = m_data.size();

    if (start_pos > data_len || end_pos > data_len || start_pos > end_pos)
        throw cppstr::Index_Out_Of_Range("Range out of bounds in cppString::substr_between.");

    if (data_len == 0 || start_pos == end_pos)
        return cppstr::cppString{};

    const pos_t limit = end_pos - start_pos;
    const pos_t take_len = (count == npos || count > limit) ? limit : count;

    if (take_len == 0)
        return cppstr::cppString{};

    return cppstr::cppString(m_data.substr(start_pos, take_len));
}

cppstr::cppString::pos_type cppstr::cppString::find(const cppstr::cppString& s, cppstr::cppString::pos_type pos) const noexcept
{
    return m_data.find(s.m_data, pos);
}

cppstr::cppString::pos_type cppstr::cppString::find(const char* s, cppstr::cppString::pos_type pos)           const noexcept
{
    return m_data.find(s, pos);
}

cppstr::cppString::pos_type cppstr::cppString::find(char ch, cppstr::cppString::pos_type pos)                 const noexcept
{
    return m_data.find(ch, pos);
}

 cppstr::cppString& cppstr::cppString::replace(pos_type pos, pos_type count, const cppString& str)
{
	cppstr::cppString::size_type data_len = m_data.size();
	if (pos > data_len)
		throw cppstr::Index_Out_Of_Range("Index out of range in replace.");

	m_data.replace(pos, count, str.m_data);

	return *this;
}

 cppstr::cppString& cppstr::cppString::replace(pos_type pos, pos_type count, const char* s)
{
	cppstr::cppString::size_type data_len = m_data.size();

	if (pos > data_len)
		throw cppstr::Index_Out_Of_Range("Index out of range in replace.");

	m_data.replace(pos, count, s);

	return *this;
}

 cppstr::cppString& cppstr::cppString::replace(pos_type pos, pos_type count, const String& str)
{
	cppstr::cppString::size_type data_len = m_data.size();

	if (pos > data_len)
		throw cppstr::Index_Out_Of_Range("Index out of range in replace.");

	m_data.replace(pos, count, str);

	return *this;
}

cppstr::cppString& cppstr::cppString::toUpper()
{
    std::transform(
        m_data.begin(),
        m_data.end(),
        m_data.begin(),
        [](unsigned char c) { return std::toupper(c); }
    );

    return *this;
}

cppstr::cppString& cppstr::cppString::toLower()
{
	std::transform(
		m_data.begin(),
		m_data.end(),
		m_data.begin(),
		[](unsigned char c) { return std::tolower(c); }
	);

	return *this;
}

cppstr::cppString cppstr::cppString::copyUpper() const
{
	cppstr::cppString result{ *this };
    result.toUpper();

    return result;
}

cppstr::cppString cppstr::cppString::copyLower() const
{
	cppstr::cppString result{ *this };
	result.toLower();

    return result;
}

cppstr::cppString& cppstr::cppString::trimLeft()
{
	m_data.erase(
        m_data.begin(),
        std::find_if(
            m_data.begin(),
            m_data.end(),
            [](unsigned char ch) { return !std::isspace(ch); }
        )
    );

	return *this;
}

cppstr::cppString& cppstr::cppString::trimRight()
{
    m_data.erase(
        std::find_if(
            m_data.rbegin(),
            m_data.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }
        ).base(),
        m_data.end()
    );
    return *this;
}

cppstr::cppString& cppstr::cppString::trim()
{
	trimLeft();
	trimRight();
	return *this;
}

cppstr::cppString& cppstr::cppString::trim(char ch)
{
	m_data.erase(
		m_data.begin(),
		std::find_if(
			m_data.begin(),
			m_data.end(),
			[ch](unsigned char c) { return c != ch; }
		)
	);

	m_data.erase(
		std::find_if(
			m_data.rbegin(),
			m_data.rend(),
			[ch](unsigned char c) { return c != ch; }
		).base(),
		m_data.end()
	);

    return *this;
}



cppstr::cppString cppstr::cppString::copyTrim() const
{
    cppstr::cppString result{ *this };
    result.trim();

    return result;
}

cppstr::cppString cppstr::cppString::copyTrim(char ch) const
{
    cppstr::cppString result{ *this };
    result.trim(ch);

    return result;
}

 cppstr::cppString& cppstr::cppString::replaceAll(const cppString& from, const cppString& to)
{
	if (from.m_data.empty())
		throw cppstr::Invalid_Argument("The 'from' string cannot be empty in replaceAll.");

	pos_type start_pos = 0;

	while ((start_pos = m_data.find(from.m_data, start_pos)) != cppstr::cppString::npos)
	{
		m_data.replace(start_pos, from.m_data.length(), to.m_data);
		start_pos += to.m_data.length();
	}

    return *this;
}

cppstr::cppString cppstr::cppString::copyReplaceAll(const cppString& from, const cppString& to)
{
	cppstr::cppString result{ *this };
	result.replaceAll(from, to);

    return result;
}

 bool cppstr::cppString::startsWith(const cppString& prefix) const
{
	if (prefix.length() > m_data.length())
		return false;

	return m_data.compare(0, prefix.length(), prefix.m_data) == 0;
}

 bool cppstr::cppString::startsWith(const char* prefix) const
{
	if (prefix == nullptr)
		throw cppstr::Invalid_Argument("The prefix string cannot be null in startsWith.");
	size_type prefix_length = std::strlen(prefix);
	if (prefix_length > m_data.length())
		return false;
	return m_data.compare(0, prefix_length, prefix) == 0;
}

 bool cppstr::cppString::startsWith(const String& prefix) const
{
	if (prefix.length() > m_data.length())
		return false;
	return m_data.compare(0, prefix.length(), prefix) == 0;
}

 bool cppstr::cppString::startsWith(char ch) const
{
	if (m_data.empty())
		return false;
	return m_data[0] == ch;
}

 bool cppstr::cppString::endsWith(const cppString& suffix) const
{
	if (suffix.length() > m_data.length())
		return false;

	return m_data.compare(m_data.length() - suffix.length(), suffix.length(), suffix.m_data) == 0;
}

bool cppstr::cppString::endsWith(const char* suffix) const
{
	if (suffix == nullptr)
		throw cppstr::Invalid_Argument("The suffix string cannot be null in endsWith.");
	size_type suffix_length = std::strlen(suffix);
	if (suffix_length > m_data.length())
		return false;
	return m_data.compare(m_data.length() - suffix_length, suffix_length, suffix) == 0;
}

 bool cppstr::cppString::endsWith(const String& suffix) const
{
	if (suffix.length() > m_data.length())
		return false;
	return m_data.compare(m_data.length() - suffix.length(), suffix.length(), suffix) == 0;

}

 bool cppstr::cppString::endsWith(char ch) const
{
	if (m_data.empty())
		return false;
	return m_data[m_data.length() - 1] == ch;
}

bool cppstr::cppString::contains(const cppString& substring) const
{
	return m_data.find(substring.m_data) != cppstr::cppString::npos;
}

bool cppstr::cppString::contains(const char* substring) const
{
	return m_data.find(substring) != cppstr::cppString::npos;
}

bool cppstr::cppString::contains(const String& substring) const
{
    return m_data.find(substring) != cppstr::cppString::npos;
}

bool cppstr::cppString::contains(char ch) const
{
    return m_data.find(ch) != cppstr::cppString::npos;
}

bool cppstr::cppString::contains(const std::initializer_list<char>& chars) const
{
	return std::any_of(chars.begin(), chars.end(), [this](char ch) {
		return m_data.find(ch) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::vector<char>& chars) const
{
	return std::any_of(chars.begin(), chars.end(), [this](char ch) {
		return m_data.find(ch) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::set<char>& chars) const
{
	return std::any_of(chars.begin(), chars.end(), [this](char ch) {
		return m_data.find(ch) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::initializer_list<String>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const String& substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::vector<String>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const String& substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::set<String>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const String& substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::initializer_list<cppString>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const cppString& substring) {
		return m_data.find(substring.m_data) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::vector<cppString>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const cppString& substring) {
		return m_data.find(substring.m_data) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::set<cppString>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const cppString& substring) {
		return m_data.find(substring.m_data) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::initializer_list<const char*>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const char* substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::vector<const char*>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const char* substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::set<const char*>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const char* substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::initializer_list<String_view>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const String_view& substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::vector<String_view>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const String_view& substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

bool cppstr::cppString::contains(const std::set<String_view>& substrings) const
{
	return std::any_of(substrings.begin(), substrings.end(), [this](const String_view& substring) {
		return m_data.find(substring) != cppstr::cppString::npos;
		});
}

void cppstr::cppString::push_back(char c)
{
    m_data.push_back(c);
}

void cppstr::cppString::pop_back()
{
    m_data.pop_back();
}

bool cppstr::cppString::matches(const cppString& pattern) const
{
    try
    {
        std::regex re{ pattern.s_str() };
        return std::regex_match(m_data, re);
    }
    catch (std::regex_error) { throw cppstr::Invalid_Argument("Invalid regex pattern in matches."); }
    catch (std::bad_alloc) { throw cppstr::Invalid_Operation("Memory allocation failed during regex matching."); }
    catch (...) { throw cppstr::Invalid_Operation("An unknown error occurred during regex matching.");  }
}

bool cppstr::cppString::matches(const char* pattern) const
{
    try
    {
		std::regex re{ pattern };
		return std::regex_match(m_data, re);
    }
	catch (std::regex_error) { throw cppstr::Invalid_Argument("Invalid regex pattern in matches."); }
	catch (std::bad_alloc) { throw cppstr::Invalid_Operation("Memory allocation failed during regex matching."); }
    catch (...) { throw cppstr::Invalid_Operation("An unknown error occurred during regex matching.");}
}

bool cppstr::cppString::matches(const String& pattern) const
{
    try
    {
		std::regex re{ pattern };
		return std::regex_match(m_data, re);
	}
    catch (std::regex_error) { throw cppstr::Invalid_Argument("Invalid regex pattern in matches."); }
    catch (std::bad_alloc) { throw cppstr::Invalid_Operation("Memory allocation failed during regex matching."); }
	catch (...) { throw cppstr::Invalid_Operation("An unknown error occurred during regex matching."); }
}

void cppstr::cppString::regexReplace(const cppString& pattern, const cppString& replacement)
{
    try
    {
		std::regex re{ pattern.s_str() };
		m_data = std::regex_replace(m_data, re, replacement.s_str());
	}
	catch (std::regex_error) { throw cppstr::Invalid_Argument("Invalid regex pattern in regexReplace."); }
	catch (std::bad_alloc) { throw cppstr::Invalid_Operation("Memory allocation failed during regex replacement."); }
    catch (...) { throw cppstr::Invalid_Operation("An unknown error occurred during regex replacement."); }
}

void cppstr::cppString::regexReplace(const char* pattern, const char* replacement)
{
	try
	{
		std::regex re{ pattern };
		m_data = std::regex_replace(m_data, re, replacement);
	}
	catch (std::regex_error) { throw cppstr::Invalid_Argument("Invalid regex pattern in regexReplace."); }
	catch (std::bad_alloc) { throw cppstr::Invalid_Operation("Memory allocation failed during regex replacement."); }
	catch (...) { throw cppstr::Invalid_Operation("An unknown error occurred during regex replacement."); }
}

void cppstr::cppString::regexReplace(const String& pattern, const String& replacement)
{
    try
    {
		std::regex re{ pattern };
		m_data = std::regex_replace(m_data, re, replacement);
    }
	catch (std::regex_error) { throw cppstr::Invalid_Argument("Invalid regex pattern in regexReplace."); }
	catch (std::bad_alloc) { throw cppstr::Invalid_Operation("Memory allocation failed during regex replacement."); }
	catch (...) { throw cppstr::Invalid_Operation("An unknown error occurred during regex replacement."); }
}

cppstr::cppString& cppstr::cppString::fromWide(const wString& wideStr)
{
	if (wideStr.empty())
	{
		m_data.clear();
		return *this;
	}

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.size(), NULL, 0, NULL, NULL);
	String str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.size(), &str[0], size_needed, NULL, NULL);

	m_data = std::move(str);

	return *this;
}

std::wstring cppstr::cppString::toWide() const
{
	if (m_data.empty())
	{
		return std::wstring{};
	}

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, m_data.c_str(), (int)m_data.size(), NULL, 0);
	std::wstring wideStr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, m_data.c_str(), (int)m_data.size(), &wideStr[0], size_needed);

	return wideStr;
}

std::vector<cppstr::cppString> cppstr::cppString::split(const cppString& delimiter) const
{
	if (delimiter.empty())
		throw cppstr::Invalid_Argument("Delimiter cannot be empty in split.");

	if (m_data.empty())
		return {};

	if (m_data.find(delimiter.m_data) == cppstr::cppString::npos)
		return { *this }; // Return the whole string if delimiter not found

	std::vector<cppstr::cppString> tokens;
	pos_type start = 0;
	pos_type end = m_data.find(delimiter.m_data);

	while (end != cppstr::cppString::npos) {
		if (end != start) { // Avoid adding empty tokens
			tokens.emplace_back(m_data.substr(start, end - start));
		}

		start = end + delimiter.m_data.length();
		end = m_data.find(delimiter.m_data, start);

		if (start >= m_data.size()) {
			break; // Prevents infinite loop if delimiter is at the end
		}
	}

	if (start < m_data.size()) {
		tokens.emplace_back(m_data.substr(start));
	}

	return tokens;
}

std::vector<cppstr::cppString> cppstr::cppString::split(const char* delimiter) const
{
	if (delimiter == nullptr || *delimiter == '\0')
		throw cppstr::Invalid_Argument("Delimiter cannot be null or empty in split.");

	if (m_data.empty())
		return {};

	if (m_data.find(delimiter) == cppstr::cppString::npos)
		return { *this }; // Return the whole string if delimiter not found

	std::vector<cppstr::cppString> tokens;
	pos_type start = 0;
	pos_type end = m_data.find(delimiter);

	while (end != cppstr::cppString::npos) {
		if (end != start) { // Avoid adding empty tokens
			tokens.emplace_back(m_data.substr(start, end - start));
		}
		start = end + std::strlen(delimiter);
		end = m_data.find(delimiter, start);
		if (start >= m_data.size()) {
			break; // Prevents infinite loop if delimiter is at the end
		}
	}

	if (start < m_data.size()) {
		tokens.emplace_back(m_data.substr(start));
	}

	return tokens;
}

std::vector<cppstr::cppString> cppstr::cppString::split(const String& delimiter) const
{
	if (delimiter.empty())
		throw cppstr::Invalid_Argument("Delimiter cannot be empty in split.");

	if (m_data.empty())
		return {};

	if (m_data.find(delimiter) == cppstr::cppString::npos)
		return { *this }; // Return the whole string if delimiter not found

	std::vector<cppstr::cppString> tokens;
	pos_type start = 0;
	pos_type end = m_data.find(delimiter);

	while (end != cppstr::cppString::npos) {
		if (end != start) { // Avoid adding empty tokens
			tokens.emplace_back(m_data.substr(start, end - start));
		}
		start = end + delimiter.length();
		end = m_data.find(delimiter, start);
		if (start >= m_data.size()) {
			break; // Prevents infinite loop if delimiter is at the end
		}
	}

	if (start < m_data.size()) {
		tokens.emplace_back(m_data.substr(start));
	}

	return tokens;
}

std::vector<cppstr::cppString> cppstr::cppString::split(char delimiter) const
{
	if (m_data.empty())
		return {};

	if (m_data.find(delimiter) == cppstr::cppString::npos)
		return { *this }; // Return the whole string if delimiter not found

	std::vector<cppstr::cppString> tokens;
	pos_type start = 0;
	pos_type end = m_data.find(delimiter);

	while (end != cppstr::cppString::npos) {
		if (end != start) { // Avoid adding empty tokens
			tokens.emplace_back(m_data.substr(start, end - start));
		}
		start = end + 1;
		end = m_data.find(delimiter, start);
		if (start >= m_data.size()) {
			break; // Prevents infinite loop if delimiter is at the end
		}
	}

	if (start < m_data.size()) {
		tokens.emplace_back(m_data.substr(start));
	}

	return tokens;
}

cppstr::cppString::size_type cppstr::cppString::findFirstOf(const cppString& chars, pos_type pos) const
{
	if (chars.empty())
		return npos;

	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in findFirstOf.");

	if (chars.size() == 1)
		return m_data.find(chars[0], pos);

	return m_data.find_first_of(chars.m_data, pos);
}

cppstr::cppString::size_type cppstr::cppString::findFirstOf(const char* chars, pos_type pos) const
{
	if (chars == nullptr || *chars == '\0')
		return npos;

	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in findFirstOf.");

	if (std::strlen(chars) == 1)
		return m_data.find(chars[0], pos);

	return m_data.find_first_of(chars, pos);
}

cppstr::cppString::size_type cppstr::cppString::findFirstOf(const String& chars, pos_type pos) const
{
	if (chars.empty())
		return npos;

	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in findFirstOf.");

	if (chars.size() == 1)
		return m_data.find(chars[0], pos);

	return m_data.find_first_of(chars, pos);
}

cppstr::cppString::size_type cppstr::cppString::findFirstOf(char ch, pos_type pos) const
{
	return m_data.find(ch, pos);
}

cppstr::cppString::size_type cppstr::cppString::findLastOf(const cppString& chars, pos_type pos) const
{
	if (chars.empty())
		return npos;

	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in findLastOf.");

	if (chars.size() == 1)
		return m_data.find_last_of(chars[0], pos);

	return m_data.find_last_of(chars.m_data, pos);
}

cppstr::cppString::size_type cppstr::cppString::findLastOf(const char* chars, pos_type pos) const
{
	if (chars == nullptr || *chars == '\0')
		return npos;

	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in findLastOf.");

	if (std::strlen(chars) == 1)
		return m_data.find_last_of(chars[0], pos);

	return m_data.find_last_of(chars, pos);
}

cppstr::cppString::size_type cppstr::cppString::findLastOf(const String& chars, pos_type pos) const
{
	if (chars.empty())
		return npos;

	if (pos > m_data.size())
		throw cppstr::Index_Out_Of_Range("Index out of range in findLastOf.");

	if (chars.size() == 1)
		return m_data.find_last_of(chars[0], pos);

	return m_data.find_last_of(chars, pos);
}

cppstr::cppString::size_type cppstr::cppString::findLastOf(char ch, pos_type pos) const
{
	return m_data.find_last_of(ch, pos);
}

cppstr::cppString::size_type cppstr::cppString::UTF8Length() const
{
	return cppstr::utf8::length(*this);
}

cppstr::cppString::size_type cppstr::cppString::UTF8CharCount() const
{
	return cppstr::utf8::length(*this);
}

cppstr::cppString cppstr::cppString::UTF8SubString(size_type start, size_type length) const
{
	return cppstr::utf8::sub(*this, start, length);
}

cppstr::cppString cppstr::cppString::UTF8SubString(cppstr::cppString::size_type start) const
{
	return cppstr::utf8::sub(*this, start);
}

cppstr::cppString cppstr::cppString::UTF8SubString(size_type start, size_type end, size_type length) const
{
	if (m_data.empty())
		throw cppstr::Invalid_Operation("Cannot call UTF8Substring on an empty string.");

	const size_type char_count = UTF8CharCount();

	if (start >= char_count || end > char_count || start >= end)
		throw cppstr::Index_Out_Of_Range("Start or end index out of range in UTF8Substring.");

	if (length == 0)
		throw cppstr::Empty_String("Length cannot be zero in UTF8Substring.");

	if (start + length > end)
		throw cppstr::Index_Out_Of_Range("Length exceeds substring bounds in UTF8Substring.");

	return cppstr::utf8::sub(*this, start, length);
}

void cppstr::cppString::swap(cppString& other) noexcept
{
	m_data.swap(other.m_data);
}

bool cppstr::cppString::is_clear() const noexcept
{
	return empty();
}

bool cppstr::cppString::is_empty() const noexcept
{
	return empty();
}

bool cppstr::cppString::is_not_empty() const noexcept
{
	return !empty();
}

bool cppstr::cppString::is_not_clear() const noexcept
{
	return !empty();
}

bool cppstr::cppString::is_whitespace() const noexcept
{

	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isspace(c); });
}

bool cppstr::cppString::is_not_whitespace() const noexcept
{
	return !is_whitespace();
}

bool cppstr::cppString::is_digit() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isdigit(c); });
}

bool cppstr::cppString::is_not_digit() const noexcept
{
	return !is_digit();
}

bool cppstr::cppString::is_alpha() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isalpha(c); });
}

bool cppstr::cppString::is_not_alpha() const noexcept
{
	return !is_alpha();
}

bool cppstr::cppString::is_alnum() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isalnum(c); });
}

bool cppstr::cppString::is_not_alnum() const noexcept
{
	return !is_alnum();
}

bool cppstr::cppString::is_lower() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::islower(c); });
}

bool cppstr::cppString::is_not_lower() const noexcept
{
	return !is_lower();
}

bool cppstr::cppString::is_upper() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isupper(c); });
}

bool cppstr::cppString::is_not_upper() const noexcept
{
	return !is_upper();
}

bool cppstr::cppString::is_ascii() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return c <= 127; });
}

bool cppstr::cppString::is_not_ascii() const noexcept
{
	return !is_ascii();
}

bool cppstr::cppString::is_printable() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isprint(c); });
}

bool cppstr::cppString::is_not_printable() const noexcept
{
	return !is_printable();
}

bool cppstr::cppString::is_graphical() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isgraph(c); });
}

bool cppstr::cppString::is_not_graphical() const noexcept
{
	return !is_graphical();
}

bool cppstr::cppString::is_punctuation() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::ispunct(c); });
}

bool cppstr::cppString::is_not_punctuation() const noexcept
{
	return !is_punctuation();
}

bool cppstr::cppString::is_control() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::iscntrl(c); });
}

bool cppstr::cppString::is_not_control() const noexcept
{
	return !is_control();
}

bool cppstr::cppString::is_hex() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) { return std::isxdigit(c); });
}

bool cppstr::cppString::is_not_hex() const noexcept
{
	return !is_hex();
}

bool cppstr::cppString::is_base64() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) {
		return (std::isalnum(c) || c == '+' || c == '/' || c == '=');
		});
}

bool cppstr::cppString::is_not_base64() const noexcept
{
	return !is_base64();
}

bool cppstr::cppString::is_utf8() const noexcept
{
	return cppstr::utf8::is_valid(*this);
}

bool cppstr::cppString::is_not_utf8() const noexcept
{
	return !is_utf8();
}

bool cppstr::cppString::is_regex_string() const noexcept
{
	return std::all_of(m_data.begin(), m_data.end(), [](unsigned char c) {
		return (std::isalnum(c) || std::ispunct(c) || std::isspace(c));
		});
}

bool cppstr::cppString::is_not_regex_string() const noexcept
{
	return !is_regex_string();
}

void cppstr::cppString::reset(const String& _data)
{
	m_data = _data;
}

void cppstr::cppString::reset(String&& _data) noexcept
{
	m_data = std::move(_data);
}

void cppstr::cppString::reset(const char* _data) noexcept
{
	m_data = _data;
}

void cppstr::cppString::reset(const cppString& _data)
{
	m_data = _data.m_data;
}

void cppstr::cppString::reset(cppString&& _data) noexcept
{
	m_data = std::move(_data.m_data);
}

void cppstr::cppString::reset()
{
	m_data.clear();
}

void cppstr::cppString::reverse()
{
	m_data = cppstr::utf8::reverse(*this).s_str();
}

cppstr::cppString::size_type cppstr::cppString::count(char ch) const
{
	if (m_data.empty())
		throw cppstr::Empty_String("Cannot count characters in an empty string.");

	size_type count = 0;

	while (true) {
		size_type pos = m_data.find(ch, count);
		if (pos == npos) {
			break;
		}
		++count;
	}

	return count;
}

cppstr::cppString::size_type cppstr::cppString::count(const cppString& str) const
{
	if (m_data.empty())
		throw cppstr::Empty_String("Cannot count substrings in an empty string.");

	if (str.empty())
		throw cppstr::Invalid_Argument("The substring cannot be empty in count.");

	if (str.size() > m_data.size())
		throw cppstr::Invalid_Argument("The substring cannot be larger than the string in count.");

	size_type count = 0;

	while (true) {
		size_type pos = m_data.find(str.m_data, count);
		if (pos == npos) {
			break;
		}
		++count;
	}

	return count;
}

cppstr::cppString::size_type cppstr::cppString::count(String_view str) const
{
	if (m_data.empty())
		throw cppstr::Empty_String("Cannot count substrings in an empty string.");

	if (str.empty())
		throw cppstr::Invalid_Argument("The substring cannot be empty in count.");

	if (str.size() > m_data.size())
		throw cppstr::Invalid_Argument("The substring cannot be larger than the string in count.");

	if (str.size() == 1)
		return count(str[0]);

	size_type count = 0;

	while (true) {
		size_type pos = m_data.find(str, count);
		if (pos == npos) {
			break;
		}
		++count;
	}

	return count;
}

void cppstr::cppString::join(cppString data)
{
	if (data.empty())
		throw cppstr::Invalid_Argument("The string to join cannot be empty.");

	if (m_data.empty())
		throw cppstr::Empty_String("Cannot join to an empty string.");

	size_t total_len = 0;
	for (const auto& part : m_data) {
		total_len += 1;
	}
	total_len += data.size() * (m_data.size() - 1);

	cppString result;
	result.reserve(total_len);

	auto it = m_data.begin();
	result += *it;
	++it;

	for (; it != m_data.end(); ++it) {
		result += data;
		result += *it;
	}

	m_data = std::move(result.m_data);
}

void cppstr::cppString::join(const String& data)
{
	if (data.empty())
		throw cppstr::Invalid_Argument("The string to join cannot be empty.");

	if (m_data.empty())
		throw cppstr::Empty_String("Cannot join to an empty string.");

	size_t total_len = 0;
	for (const auto& part : m_data) {
		total_len += 1;
	}
	total_len += data.size() * (m_data.size() - 1);

	cppString result;
	result.reserve(total_len);

	auto it = m_data.begin();
	result += *it;
	++it;

	for (; it != m_data.end(); ++it) {
		result += data;
		result += *it;
	}

	m_data = std::move(result.m_data);
}

void cppstr::cppString::join(const char* data)
{
	if (data == nullptr || *data == '\0')
		throw cppstr::Invalid_Argument("The string to join cannot be null or empty.");

	if (m_data.empty())
		throw cppstr::Empty_String("Cannot join to an empty string.");

	size_t total_len = 0;
	for (const auto& part : m_data) {
		total_len += 1;
	}
	total_len += strlen(data) * (m_data.size() - 1);

	cppString result;
	result.reserve(total_len);

	auto it = m_data.begin();
	result += *it;
	++it;

	for (; it != m_data.end(); ++it) {
		result += data;
		result += *it;
	}

	m_data = std::move(result.m_data);
}

void cppstr::cppString::join(char data)
{
	if (m_data.empty())
		throw cppstr::Empty_String("Cannot join to an empty string.");

	size_t total_len = 0;
	for (const auto& part : m_data) {
		total_len += 1;
	}
	total_len += 1 * (m_data.size() - 1);

	cppString result;
	result.reserve(total_len);

	auto it = m_data.begin();
	result += *it;
	++it;

	for (; it != m_data.end(); ++it) {
		result += data;
		result += *it;
	}

	m_data = std::move(result.m_data);
}

cppstr::cppString cppstr::cppString::create()
{
    return cppString{ "" };
}

istream& operator>>(istream& is, cppstr::cppString& str)
{
    String data;

    is >> data;
    str = data;

    return is;
}

ostream& operator<<(ostream& os, const cppstr::cppString& str)
{
	return os << str.s_str();
}

std::istream& cppstr::getline(std::istream& is, cppString& str, char delim)
{
	std::getline(is, str.m_data, delim);
	return is;
}

cppstr::cppString cppstr::operator""_sp(const char* data, size_t size)
{
	return cppString(data, size);
}

cppstr::Invalid_Format::Invalid_Format(const String& msg) : String_Error(msg) {}

cppstr::Invalid_Format::Invalid_Format(const char* msg) : String_Error(msg ? msg : "One or more errors occurred during the formatting operation.") {}

cppstr::cppString::size_type cppstr::cppString::find_first_not_of(const cppString& chars, pos_type pos) const noexcept
{
	return m_data.find_first_not_of(chars.m_data, pos);
}

cppstr::cppString::size_type cppstr::cppString::find_first_not_of(const char* chars, pos_type pos) const noexcept
{
	return m_data.find_first_not_of(chars, pos);
}

cppstr::cppString::size_type cppstr::cppString::find_first_not_of(char ch, pos_type pos) const noexcept
{
	return m_data.find_first_not_of(ch, pos);
}

cppstr::cppString::size_type cppstr::cppString::find_last_not_of(const cppString& chars, pos_type pos) const noexcept
{
	return m_data.find_last_not_of(chars.m_data, pos);
}

cppstr::cppString::size_type cppstr::cppString::find_last_not_of(const char* chars, pos_type pos) const noexcept
{
	return m_data.find_last_not_of(chars, pos);
}

cppstr::cppString::size_type cppstr::cppString::find_last_not_of(char ch, pos_type pos) const noexcept
{
	return m_data.find_last_not_of(ch, pos);
}

void cppstr::swap(cppString& lhs, cppString& rhs) noexcept
{
	lhs.swap(rhs);
}
