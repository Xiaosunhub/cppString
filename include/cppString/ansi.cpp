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

#include "ansi.h"
#include "utf8.h"
#include <algorithm>

namespace
{
	const char ESC = '\x1b';

	// Byte length of a recognised escape sequence starting at pos, or 0.
	std::size_t escape_length(const String& s, std::size_t pos) noexcept
	{
		if (pos + 1 >= s.size() || s[pos] != ESC)
			return 0;

		if (s[pos + 1] == '[')
		{
			// CSI: parameter/intermediate bytes 0x20..0x3F, final 0x40..0x7E.
			std::size_t cursor = pos + 2;

			while (cursor < s.size()
				&& !(static_cast<unsigned char>(s[cursor]) >= 0x40
					&& static_cast<unsigned char>(s[cursor]) <= 0x7E))
			{
				++cursor;
			}

			return (cursor < s.size() ? cursor + 1 : s.size()) - pos;
		}

		if (s[pos + 1] == ']')
		{
			// OSC: terminated by BEL or ST (ESC backslash).
			std::size_t cursor = pos + 2;

			while (cursor < s.size())
			{
				if (s[cursor] == '\a')
					return cursor + 1 - pos;

				if (s[cursor] == ESC && cursor + 1 < s.size() && s[cursor + 1] == '\\')
					return cursor + 2 - pos;

				++cursor;
			}

			return s.size() - pos;
		}

		return 0;
	}

	std::size_t codepoint_length(unsigned char lead) noexcept
	{
		if (lead < 0x80) return 1;
		if ((lead & 0xE0) == 0xC0) return 2;
		if ((lead & 0xF0) == 0xE0) return 3;
		if ((lead & 0xF8) == 0xF0) return 4;
		return 1; // continuation byte or invalid lead: treat as one column
	}

	std::size_t chunk_width(const String& chunk)
	{
		return sp::utf8::display_width(sp::cppString{ chunk });
	}

	String sgr_prefix(const std::vector<int>& codes)
	{
		String out;
		out += ESC;
		out += '[';

		for (std::size_t i = 0; i < codes.size(); ++i)
		{
			if (i != 0)
				out += ';';

			out += std::to_string(codes[i]);
		}

		out += 'm';
		return out;
	}
}

sp::cppString sp::ansi::reset()
{
	return cppString{ "\x1b[0m" };
}

sp::cppString sp::ansi::strip(const cppString& s)
{
	const String& src = s.s_str();
	String out;
	out.reserve(src.size());

	for (std::size_t i = 0; i < src.size();)
	{
		const std::size_t length = escape_length(src, i);

		if (length != 0)
		{
			i += length;
		}
		else
		{
			out += src[i];
			++i;
		}
	}

	return cppString(std::move(out));
}

bool sp::ansi::has_ansi(const cppString& s) noexcept
{
	const String& src = s.s_str();

	for (std::size_t i = 0; i + 1 < src.size(); ++i)
	{
		if (escape_length(src, i) != 0)
			return true;
	}

	return false;
}

std::size_t sp::ansi::visible_width(const cppString& s)
{
	return utf8::display_width(strip(s));
}

sp::cppString sp::ansi::truncate(const cppString& s, std::size_t max_width,
	const cppString& suffix)
{
	const String& src = s.s_str();
	const String tail = suffix.s_str();
	const std::size_t tail_width = tail.empty() ? 0 : chunk_width(tail);
	const std::size_t reserve = tail_width <= max_width ? tail_width : 0;
	const std::size_t budget = max_width - reserve;

	String out;
	std::size_t width = 0;
	bool truncated = false;

	for (std::size_t i = 0; i < src.size();)
	{
		const std::size_t length = escape_length(src, i);

		if (length != 0)
		{
			// Keep colour/attribute control sequences verbatim.
			out.append(src, i, length);
			i += length;
			continue;
		}

		const std::size_t bytes = (std::min)(
			codepoint_length(static_cast<unsigned char>(src[i])),
			src.size() - i);

		const String chunk = src.substr(i, bytes);
		const std::size_t w = chunk_width(chunk);

		if (width + w > budget)
		{
			truncated = true;
			break;
		}

		out += chunk;
		width += w;
		i += bytes;
	}

	if (truncated)
	{
		out += tail;

		if (has_ansi(s))
			out += reset().s_str();
	}

	return cppString(std::move(out));
}

sp::cppString sp::ansi::pad(const cppString& s, std::size_t width, Align align, char fill)
{
	const std::size_t current = visible_width(s);

	if (current >= width)
		return s;

	const std::size_t padding = width - current;
	std::size_t left = 0;
	std::size_t right = 0;

	if (align == Align::Left)
	{
		right = padding;
	}
	else if (align == Align::Center)
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

sp::cppString sp::ansi::style(const cppString& s, const std::vector<int>& codes)
{
	String out = sgr_prefix(codes);
	out += s.s_str();
	out += "\x1b[0m";
	return cppString(std::move(out));
}

sp::cppString sp::ansi::fg(const cppString& s, int color256)
{
	return style(s, { 38, 5, color256 });
}

sp::cppString sp::ansi::rgb(const cppString& s, int r, int g, int b)
{
	return style(s, { 38, 2, r, g, b });
}

sp::cppString sp::ansi::bold(const cppString& s)
{
	return style(s, { 1 });
}

sp::cppString sp::ansi::underline(const cppString& s)
{
	return style(s, { 4 });
}

sp::cppString sp::ansi::inverse(const cppString& s)
{
	return style(s, { 7 });
}
