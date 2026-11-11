// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "utf8.h"
#include <algorithm>

namespace
{
	std::size_t sequence_length(unsigned char lead)
	{
		if ((lead & 0x80) == 0x00) return 1;
		if ((lead & 0xE0) == 0xC0) return 2;
		if ((lead & 0xF0) == 0xE0) return 3;
		if ((lead & 0xF8) == 0xF0) return 4;
		return 1; // invalid lead byte, treated as a single byte
	}
}

bool sp::utf8::is_valid(const cppString& s)
{
	const auto* bytes = reinterpret_cast<const unsigned char*>(s.data());
	const std::size_t len = s.size();

	std::size_t i = 0;
	while (i < len)
	{
		const unsigned char lead = bytes[i];

		if (lead <= 0x7F)
		{
			++i;
			continue;
		}

		std::size_t extra = 0;
		unsigned char second_min = 0x80;
		unsigned char second_max = 0xBF;

		if (lead >= 0xC2 && lead <= 0xDF)      extra = 1;
		else if (lead == 0xE0)                 { extra = 2; second_min = 0xA0; }
		else if (lead >= 0xE1 && lead <= 0xEC) extra = 2;
		else if (lead == 0xED)                 { extra = 2; second_max = 0x9F; }
		else if (lead >= 0xEE && lead <= 0xEF) extra = 2;
		else if (lead == 0xF0)                 { extra = 3; second_min = 0x90; }
		else if (lead >= 0xF1 && lead <= 0xF3) extra = 3;
		else if (lead == 0xF4)                 { extra = 3; second_max = 0x8F; }
		else return false;

		if (i + extra >= len)
			return false;

		if (bytes[i + 1] < second_min || bytes[i + 1] > second_max)
			return false;

		for (std::size_t j = 2; j <= extra; ++j)
		{
			if ((bytes[i + j] & 0xC0) != 0x80)
				return false;
		}

		i += extra + 1;
	}

	return true;
}

std::size_t sp::utf8::length(const cppString& s)
{
	const auto* bytes = reinterpret_cast<const unsigned char*>(s.data());
	const std::size_t len = s.size();

	std::size_t count = 0;
	for (std::size_t i = 0; i < len; ++i)
	{
		if ((bytes[i] & 0xC0) != 0x80) // not a continuation byte
			++count;
	}

	return count;
}

std::size_t sp::utf8::offset(const cppString& s, std::size_t char_index)
{
	const auto* bytes = reinterpret_cast<const unsigned char*>(s.data());
	const std::size_t len = s.size();

	std::size_t i = 0;
	std::size_t count = 0;

	while (i < len)
	{
		if (count == char_index)
			return i;

		i += sequence_length(bytes[i]);
		++count;
	}

	return (count == char_index) ? len : cppString::npos;
}

sp::cppString sp::utf8::sub(const cppString& s, std::size_t start)
{
	if (s.is_empty())
		throw sp::Empty_String("Cannot call utf8::sub on an empty string.");

	const std::size_t count = length(s);

	if (start >= count)
		throw sp::Index_Out_Of_Range("Start index out of range in utf8::sub.");

	if (start == 0)
		return s;

	return cppString(s.s_str().substr(offset(s, start)));
}

sp::cppString sp::utf8::sub(const cppString& s, std::size_t start, std::size_t len)
{
	if (s.is_empty())
		throw sp::Empty_String("Cannot call utf8::sub on an empty string.");

	const std::size_t count = length(s);

	if (start >= count)
		throw sp::Index_Out_Of_Range("Start index out of range in utf8::sub.");

	if (len == 0)
		return cppString{};

	if (start + len > count)
		throw sp::Index_Out_Of_Range("Length exceeds string bounds in utf8::sub.");

	const std::size_t start_byte = offset(s, start);
	const std::size_t end_byte = offset(s, start + len);

	return cppString(s.s_str().substr(start_byte, end_byte - start_byte));
}

std::vector<sp::cppString> sp::utf8::chars(const cppString& s)
{
	std::vector<cppString> result;

	const String& str = s.s_str();
	std::size_t i = 0;

	while (i < str.size())
	{
		std::size_t step = sequence_length(static_cast<unsigned char>(str[i]));
		if (i + step > str.size())
			step = 1; // truncated tail: keep the byte as-is

		result.emplace_back(str.substr(i, step));
		i += step;
	}

	return result;
}

std::vector<char32_t> sp::utf8::codepoints(const cppString& s)
{
	std::vector<char32_t> result;

	const String& str = s.s_str();
	std::size_t i = 0;

	while (i < str.size())
	{
		const unsigned char lead = static_cast<unsigned char>(str[i]);
		const std::size_t step = sequence_length(lead);

		if (step == 1 && lead > 0x7F)
			throw sp::Invalid_Conversion("Invalid UTF-8 lead byte in utf8::codepoints.");

		if (i + step > str.size())
			throw sp::Invalid_Conversion("Truncated UTF-8 sequence in utf8::codepoints.");

		char32_t cp = 0;
		switch (step)
		{
		case 1: cp = lead;         break;
		case 2: cp = lead & 0x1F;  break;
		case 3: cp = lead & 0x0F;  break;
		default: cp = lead & 0x07; break;
		}

		for (std::size_t j = 1; j < step; ++j)
		{
			const unsigned char c = static_cast<unsigned char>(str[i + j]);
			if ((c & 0xC0) != 0x80)
				throw sp::Invalid_Conversion("Invalid UTF-8 continuation byte in utf8::codepoints.");

			cp = (cp << 6) | (c & 0x3F);
		}

		result.push_back(cp);
		i += step;
	}

	return result;
}

sp::cppString sp::utf8::from_codepoint(char32_t cp)
{
	String out;

	if (cp <= 0x7F)
	{
		out += static_cast<char>(cp);
	}
	else if (cp <= 0x7FF)
	{
		out += static_cast<char>(0xC0 | (cp >> 6));
		out += static_cast<char>(0x80 | (cp & 0x3F));
	}
	else if (cp <= 0xFFFF)
	{
		if (cp >= 0xD800 && cp <= 0xDFFF)
			throw sp::Invalid_Argument("Surrogate code points cannot be encoded.");

		out += static_cast<char>(0xE0 | (cp >> 12));
		out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (cp & 0x3F));
	}
	else if (cp <= 0x10FFFF)
	{
		out += static_cast<char>(0xF0 | (cp >> 18));
		out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
		out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (cp & 0x3F));
	}
	else
	{
		throw sp::Invalid_Argument("Code point out of Unicode range.");
	}

	return cppString(out);
}

sp::cppString sp::utf8::reverse(const cppString& s)
{
	auto parts = chars(s);
	std::reverse(parts.begin(), parts.end());

	String out;
	for (const auto& part : parts)
		out += part.s_str();

	return cppString(out);
}

namespace
{
	bool is_wide(char32_t cp) noexcept
	{
		return (cp >= 0x1100 && cp <= 0x115F)      // Hangul Jamo
			|| (cp >= 0x2E80 && cp <= 0x303E)      // CJK Radicals, Kangxi, Ideographic Description
			|| (cp >= 0x3041 && cp <= 0x33FF)      // Hiragana, Katakana, CJK Symbols
			|| (cp >= 0x3400 && cp <= 0x4DBF)      // CJK Ext A
			|| (cp >= 0x4E00 && cp <= 0x9FFF)      // CJK Unified Ideographs
			|| (cp >= 0xA000 && cp <= 0xA4CF)      // Yi
			|| (cp >= 0xAC00 && cp <= 0xD7A3)      // Hangul Syllables
			|| (cp >= 0xF900 && cp <= 0xFAFF)      // CJK Compatibility Ideographs
			|| (cp >= 0xFE30 && cp <= 0xFE4F)      // CJK Compatibility Forms
			|| (cp >= 0xFF00 && cp <= 0xFF60)      // Fullwidth Forms
			|| (cp >= 0xFFE0 && cp <= 0xFFE6)      // Fullwidth signs
			|| (cp >= 0x1F300 && cp <= 0x1F64F)    // Emoji
			|| (cp >= 0x20000 && cp <= 0x3FFFD);   // CJK Ext B+
	}

	bool is_zero_width(char32_t cp) noexcept
	{
		return cp < 0x20                           // C0 control characters
			|| cp == 0x7F                          // DEL
			|| (cp >= 0x0300 && cp <= 0x036F)      // combining diacritical marks
			|| (cp >= 0x200B && cp <= 0x200F)      // zero-width space / joiners
			|| (cp >= 0xFE00 && cp <= 0xFE0F)      // variation selectors
			|| cp == 0xFEFF;                       // BOM / zero-width no-break space
	}
}

std::size_t sp::utf8::display_width(const cppString& s)
{
	std::size_t width = 0;

	for (const char32_t cp : codepoints(s))
	{
		if (is_zero_width(cp))
			continue;

		width += is_wide(cp) ? 2 : 1;
	}

	return width;
}

// ---- display-width helpers (5.1) ----

sp::cppString sp::utf8::truncate_by_width(const cppString& s, std::size_t max_width,
	const cppString& suffix)
{
	const auto characters = chars(s);
	const std::size_t suffix_width = display_width(suffix);
	const bool fits = !suffix.empty() && suffix_width <= max_width;
	const std::size_t budget = fits ? max_width - suffix_width : max_width;

	String out;
	std::size_t width = 0;
	bool cut = false;

	for (const auto& character : characters)
	{
		const std::size_t w = display_width(character);

		if (width + w > budget)
		{
			cut = true;
			break;
		}

		out += character.s_str();
		width += w;
	}

	if (cut && fits)
		out += suffix.s_str();

	return cppString(std::move(out));
}

sp::cppString sp::utf8::pad_by_width(const cppString& s, std::size_t width,
	Alignment align, char fill)
{
	const std::size_t current = display_width(s);

	if (current >= width)
		return s;

	const std::size_t padding = width - current;
	std::size_t left = 0;
	std::size_t right = 0;

	if (align == Alignment::Left)
	{
		right = padding;
	}
	else if (align == Alignment::Center)
	{
		left = padding / 2;
		right = padding - left;
	}
	else
	{
		left = padding;
	}

	String out(left, fill);
	out += s.s_str();
	out += String(right, fill);
	return cppString(std::move(out));
}


bool sp::utf8::has_bom(const cppString& s) noexcept
{
	const String& data = s.s_str();
	return data.size() >= 3
		&& static_cast<unsigned char>(data[0]) == 0xEF
		&& static_cast<unsigned char>(data[1]) == 0xBB
		&& static_cast<unsigned char>(data[2]) == 0xBF;
}

sp::cppString sp::utf8::strip_bom(const cppString& s)
{
	if (!has_bom(s))
		return s;
	return cppString{ s.s_str().substr(3) };
}

sp::cppString sp::utf8::add_bom(const cppString& s)
{
	if (has_bom(s))
		return s;
	String out{};
	out += static_cast<char>(0xEF);
	out += static_cast<char>(0xBB);
	out += static_cast<char>(0xBF);
	out += s.s_str();
	return cppString{ std::move(out) };
}
