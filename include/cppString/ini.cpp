// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "ini.h"
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>

namespace
{
	bool is_space(char c) noexcept
	{
		return c == ' ' || c == '\t' || c == '\r' || c == '\n';
	}

	String trim(const String& s)
	{
		std::size_t first = 0;

		while (first < s.size() && is_space(s[first]))
			++first;

		std::size_t last = s.size();

		while (last > first && is_space(s[last - 1]))
			--last;

		return s.substr(first, last - first);
	}

	String ascii_lower(String s)
	{
		for (char& c : s)
		{
			if (c >= 'A' && c <= 'Z')
				c = static_cast<char>(c - 'A' + 'a');
		}

		return s;
	}

	String decode_quoted(const String& raw)
	{
		if (raw.size() < 2)
			return raw;

		const char quote = raw.front();

		if ((quote != '"' && quote != '\'')
			|| raw.back() != quote)
		{
			return raw;
		}

		String out;

		for (std::size_t i = 1; i + 1 < raw.size(); ++i)
		{
			if (raw[i] == '\\' && i + 2 < raw.size())
			{
				++i;

				switch (raw[i])
				{
				case 'n': out += '\n'; break;
				case 'r': out += '\r'; break;
				case 't': out += '\t'; break;
				case '\\': out += '\\'; break;
				case '"': out += '"'; break;
				case '\'': out += '\''; break;
				default: out += raw[i]; break;
				}
			}
			else
			{
				out += raw[i];
			}
		}

		return out;
	}

	bool needs_quotes(const String& value)
	{
		if (value.empty() || is_space(value.front()) || is_space(value.back()))
			return true;

		for (const char c : value)
		{
			if (c == '"' || c == '\'' || c == ';' || c == '#' || c == '='
				|| c == '\n' || c == '\r')
			{
				return true;
			}
		}

		return false;
	}

	String encode_value(const String& value)
	{
		if (!needs_quotes(value))
			return value;

		String out;
		out += '"';

		for (const char c : value)
		{
			switch (c)
			{
			case '\\': out += "\\\\"; break;
			case '"': out += "\\\""; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default: out += c; break;
			}
		}

		out += '"';
		return out;
	}
}

sp::ini::Document::Document()
{
	m_sections.emplace_back(cppString{}, Section{});
}

sp::ini::Document::Section* sp::ini::Document::find_section(const cppString& name) noexcept
{
	for (auto& entry : m_sections)
	{
		if (entry.first == name)
			return &entry.second;
	}

	return nullptr;
}

const sp::ini::Document::Section* sp::ini::Document::find_section(const cppString& name) const noexcept
{
	for (const auto& entry : m_sections)
	{
		if (entry.first == name)
			return &entry.second;
	}

	return nullptr;
}

sp::ini::Document::Section& sp::ini::Document::make_section(const cppString& name)
{
	if (Section* existing = find_section(name))
		return *existing;

	m_sections.emplace_back(name, Section{});
	return m_sections.back().second;
}

std::vector<sp::cppString> sp::ini::Document::sections() const
{
	std::vector<cppString> result;

	for (const auto& entry : m_sections)
	{
		if (!entry.first.empty())
			result.push_back(entry.first);
	}

	return result;
}

bool sp::ini::Document::has_section(const cppString& name) const noexcept
{
	return find_section(name) != nullptr;
}

sp::ini::Document::Section sp::ini::Document::entries(const cppString& name) const
{
	const Section* section = find_section(name);

	if (!section)
		throw sp::Invalid_Argument("Unknown INI section in Document::entries: " + name.s_str());

	return *section;
}

bool sp::ini::Document::has(const cppString& key) const noexcept
{
	return has(cppString{}, key);
}

bool sp::ini::Document::has(const cppString& section, const cppString& key) const noexcept
{
	const Section* s = find_section(section);

	if (!s)
		return false;

	for (const auto& entry : *s)
	{
		if (entry.first == key)
			return true;
	}

	return false;
}

sp::cppString sp::ini::Document::get_global(const cppString& key, const cppString& fallback) const
{
	return get(cppString{}, key, fallback);
}

sp::cppString sp::ini::Document::get(const cppString& section, const cppString& key,
	const cppString& fallback) const
{
	const Section* s = find_section(section);

	if (s)
	{
		for (const auto& entry : *s)
		{
			if (entry.first == key)
				return entry.second;
		}
	}

	return fallback;
}

long long sp::ini::Document::get_int_global(const cppString& key, long long fallback) const
{
	return get_int(cppString{}, key, fallback);
}

long long sp::ini::Document::get_int(const cppString& section, const cppString& key,
	long long fallback) const
{
	const cppString value = get(section, key, cppString{});

	if (value.empty())
		return fallback;

	const String& s = value.s_str();
	char* end = nullptr;
	errno = 0;
	const long long result = std::strtoll(s.c_str(), &end, 10);

	if (errno == ERANGE || end != s.c_str() + s.size())
		throw sp::Invalid_Conversion("Not an integer in INI value: " + s);

	return result;
}

double sp::ini::Document::get_double_global(const cppString& key, double fallback) const
{
	return get_double(cppString{}, key, fallback);
}

double sp::ini::Document::get_double(const cppString& section, const cppString& key,
	double fallback) const
{
	const cppString value = get(section, key, cppString{});

	if (value.empty())
		return fallback;

	const String& s = value.s_str();
	char* end = nullptr;
	errno = 0;
	const double result = std::strtod(s.c_str(), &end);

	if (errno == ERANGE || end != s.c_str() + s.size())
		throw sp::Invalid_Conversion("Not a number in INI value: " + s);

	return result;
}

bool sp::ini::Document::get_bool_global(const cppString& key, bool fallback) const
{
	return get_bool(cppString{}, key, fallback);
}

bool sp::ini::Document::get_bool(const cppString& section, const cppString& key,
	bool fallback) const
{
	const cppString value = get(section, key, cppString{});

	if (value.empty())
		return fallback;

	const String s = ascii_lower(value.s_str());

	if (s == "1" || s == "true" || s == "yes" || s == "on" || s == "y" || s == "t")
		return true;

	if (s == "0" || s == "false" || s == "no" || s == "off" || s == "n" || s == "f")
		return false;

	throw sp::Invalid_Conversion("Not a boolean in INI value: " + s);
}

void sp::ini::Document::set(const cppString& key, const cppString& value)
{
	set(cppString{}, key, value);
}

void sp::ini::Document::set(const cppString& section, const cppString& key,
	const cppString& value)
{
	Section& s = make_section(section);

	for (auto& entry : s)
	{
		if (entry.first == key)
		{
			entry.second = value;
			return;
		}
	}

	s.emplace_back(key, value);
}

bool sp::ini::Document::erase(const cppString& key)
{
	return erase(cppString{}, key);
}

bool sp::ini::Document::erase(const cppString& section, const cppString& key)
{
	Section* s = find_section(section);

	if (!s)
		return false;

	const auto old_size = s->size();

	s->erase(std::remove_if(s->begin(), s->end(),
		[&](const Entry& entry) { return entry.first == key; }), s->end());

	return s->size() != old_size;
}

bool sp::ini::Document::erase_section(const cppString& name)
{
	if (name.empty())
		return false; // the global section always exists

	const auto old_size = m_sections.size();

	m_sections.erase(std::remove_if(m_sections.begin(), m_sections.end(),
		[&](const std::pair<cppString, Section>& entry) { return entry.first == name; }),
		m_sections.end());

	return m_sections.size() != old_size;
}

sp::cppString sp::ini::Document::dump() const
{
	String out;

	for (const auto& section : m_sections)
	{
		if (!section.first.empty())
		{
			if (!out.empty())
				out += '\n';

			out += '[';
			out += section.first.s_str();
			out += "]\n";
		}

		for (const auto& entry : section.second)
		{
			out += entry.first.s_str();
			out += " = ";
			out += encode_value(entry.second.s_str());
			out += '\n';
		}
	}

	return cppString(std::move(out));
}

sp::ini::Document sp::ini::parse(const cppString& text)
{
	Document doc;
	const String& src = text.s_str();
	cppString current_section{};
	std::size_t line_no = 0;
	std::size_t pos = 0;

	while (pos <= src.size())
	{
		const std::size_t end = src.find('\n', pos);
		String line = src.substr(pos, end == String::npos ? String::npos : end - pos);

		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		++line_no;

		const String trimmed = trim(line);

		if (!trimmed.empty() && trimmed.front() != ';' && trimmed.front() != '#')
		{
			if (trimmed.front() == '[')
			{
				const std::size_t close = trimmed.find(']');

				if (close == String::npos)
					throw sp::Invalid_Format("Unterminated INI section at line "
						+ std::to_string(line_no));

				current_section = cppString{ trim(trimmed.substr(1, close - 1)) };
				doc.set(current_section, cppString{}, cppString{}); // placeholder handled below
				// Remove the placeholder key: a section with no keys still exists.
				doc.erase(current_section, cppString{});
			}
			else
			{
				std::size_t sep = trimmed.find('=');
				const std::size_t colon = trimmed.find(':');

				if (colon != String::npos && (sep == String::npos || colon < sep))
					sep = colon;

				if (sep == String::npos)
					throw sp::Invalid_Format("INI line without '=' or ':' at line "
						+ std::to_string(line_no));

				const String key = trim(trimmed.substr(0, sep));
				String value = trim(trimmed.substr(sep + 1));

				if (!value.empty() && (value.front() == '"' || value.front() == '\''))
				{
					const char quote = value.front();
					std::size_t k = 1;

					while (k < value.size())
					{
						if (value[k] == '\\')
						{
							k += 2;
							continue;
						}

						if (value[k] == quote)
						{
							value.resize(k + 1);
							break;
						}

						++k;
					}
				}
				else
				{
					for (std::size_t k = 0; k < value.size(); ++k)
					{
						if ((value[k] == ';' || value[k] == '#')
							&& (k == 0 || is_space(value[k - 1])))
						{
							value.resize(k);
							break;
						}
					}

					value = trim(value);
				}

				doc.set(current_section, cppString{ key },
					cppString{ decode_quoted(value) });
			}
		}

		if (end == String::npos)
			break;

		pos = end + 1;
	}

	return doc;
}
