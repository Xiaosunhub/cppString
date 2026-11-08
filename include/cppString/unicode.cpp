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

#include "unicode.h"
#include "utf8.h"

namespace
{
	struct Decomp
	{
		char32_t lower;   // precomposed lowercase code point
		char32_t base;    // uppercase ASCII base letter
		char32_t mark;    // combining mark
	};

	// Only the lowercase form is stored; uppercase maps to it via fold_one.
	constexpr Decomp DECOMP[] =
	{
		{ 0x00E0, 'A', 0x0300 }, { 0x00E1, 'A', 0x0301 },
		{ 0x00E2, 'A', 0x0302 }, { 0x00E3, 'A', 0x0303 },
		{ 0x00E4, 'A', 0x0308 }, { 0x00E5, 'A', 0x030A },
		{ 0x00E7, 'C', 0x0327 },
		{ 0x00E8, 'E', 0x0300 }, { 0x00E9, 'E', 0x0301 },
		{ 0x00EA, 'E', 0x0302 }, { 0x00EB, 'E', 0x0308 },
		{ 0x00EC, 'I', 0x0300 }, { 0x00ED, 'I', 0x0301 },
		{ 0x00EE, 'I', 0x0302 }, { 0x00EF, 'I', 0x0308 },
		{ 0x00F1, 'N', 0x0303 },
		{ 0x00F2, 'O', 0x0300 }, { 0x00F3, 'O', 0x0301 },
		{ 0x00F4, 'O', 0x0302 }, { 0x00F5, 'O', 0x0303 },
		{ 0x00F6, 'O', 0x0308 },
		{ 0x00F9, 'U', 0x0300 }, { 0x00FA, 'U', 0x0301 },
		{ 0x00FB, 'U', 0x0302 }, { 0x00FC, 'U', 0x0308 },
		{ 0x00FD, 'Y', 0x0301 }, { 0x00FF, 'Y', 0x0308 },
		{ 0x0101, 'A', 0x0304 }, { 0x0103, 'A', 0x0306 },
		{ 0x0105, 'A', 0x0328 }, { 0x0107, 'C', 0x0301 },
		{ 0x0109, 'C', 0x0302 }, { 0x010B, 'C', 0x0307 },
		{ 0x010D, 'C', 0x030C }, { 0x010F, 'D', 0x030C },
		{ 0x0113, 'E', 0x0304 }, { 0x0117, 'E', 0x0307 },
		{ 0x0119, 'E', 0x0328 }, { 0x011B, 'E', 0x030C },
		{ 0x011D, 'G', 0x0302 }, { 0x011F, 'G', 0x0306 },
		{ 0x0121, 'G', 0x0307 }, { 0x0123, 'G', 0x0327 },
		{ 0x0125, 'H', 0x0302 }, { 0x0129, 'I', 0x0303 },
		{ 0x012B, 'I', 0x0304 }, { 0x012F, 'I', 0x0328 },
		{ 0x0135, 'J', 0x0302 }, { 0x0137, 'K', 0x0327 },
		{ 0x013C, 'L', 0x0327 }, { 0x0144, 'N', 0x0301 },
		{ 0x0146, 'N', 0x0327 }, { 0x0148, 'N', 0x030C },
		{ 0x014D, 'O', 0x0304 }, { 0x0151, 'O', 0x030B },
		{ 0x0155, 'R', 0x0301 }, { 0x0159, 'R', 0x030C },
		{ 0x015B, 'S', 0x0301 }, { 0x015D, 'S', 0x0302 },
		{ 0x015F, 'S', 0x0327 }, { 0x0161, 'S', 0x030C },
		{ 0x0163, 'T', 0x0327 }, { 0x0165, 'T', 0x030C },
		{ 0x0169, 'U', 0x0303 }, { 0x016B, 'U', 0x0304 },
		{ 0x016F, 'U', 0x030A }, { 0x0171, 'U', 0x030B },
		{ 0x0173, 'U', 0x0328 }, { 0x0175, 'W', 0x0302 },
		{ 0x0177, 'Y', 0x0302 }, { 0x017A, 'Z', 0x0301 },
		{ 0x017C, 'Z', 0x0307 }, { 0x017E, 'Z', 0x030C }
	};

	struct Mapping
	{
		char32_t cp;
		const char* text;
	};

	// Letters without a canonical decomposition.
	constexpr Mapping SPECIAL[] =
	{
		{ 0x00DF, "ss" }, { 0x00C6, "AE" }, { 0x00E6, "ae" },
		{ 0x0152, "OE" }, { 0x0153, "oe" },
		{ 0x00D8, "O" },  { 0x00F8, "o" },
		{ 0x00D0, "D" },  { 0x00F0, "d" },
		{ 0x00DE, "Th" }, { 0x00FE, "th" },
		{ 0x0110, "D" },  { 0x0111, "d" },
		{ 0x0141, "L" },  { 0x0142, "l" },
		{ 0x0130, "I" },  { 0x0131, "i" },  { 0x0126, "H" },  { 0x0127, "h" },
		{ 0x0166, "T" },  { 0x0167, "t" },
		{ 0x014A, "N" },  { 0x014B, "n" },
		// Greek letters with tonos / dialytika (no NFD entry above).
		{ 0x0386, "A" },  { 0x0388, "E" },  { 0x0389, "I" },
		{ 0x038A, "I" },  { 0x038C, "O" },  { 0x038E, "Y" },  { 0x038F, "O" },
		{ 0x03AC, "a" },  { 0x03AD, "e" },  { 0x03AE, "i" },
		{ 0x03AF, "i" },  { 0x03CC, "o" },  { 0x03CD, "y" },  { 0x03CE, "o" },
		{ 0x03CA, "i" },  { 0x03CB, "y" },  { 0x0390, "i" },  { 0x03B0, "y" }
	};

	// Greek and Cyrillic, keyed by the folded (lowercase) code point.
	constexpr Mapping LETTERS[] =
	{
		{ 0x03B1, "a" }, { 0x03B2, "b" }, { 0x03B3, "g" }, { 0x03B4, "d" },
		{ 0x03B5, "e" }, { 0x03B6, "z" }, { 0x03B7, "i" }, { 0x03B8, "th" },
		{ 0x03B9, "i" }, { 0x03BA, "k" }, { 0x03BB, "l" }, { 0x03BC, "m" },
		{ 0x03BD, "n" }, { 0x03BE, "x" }, { 0x03BF, "o" }, { 0x03C0, "p" },
		{ 0x03C1, "r" }, { 0x03C2, "s" }, { 0x03C3, "s" }, { 0x03C4, "t" },
		{ 0x03C5, "y" }, { 0x03C6, "ph" }, { 0x03C7, "ch" }, { 0x03C8, "ps" },
		{ 0x03C9, "o" },
		{ 0x0430, "a" }, { 0x0431, "b" }, { 0x0432, "v" }, { 0x0433, "g" },
		{ 0x0434, "d" }, { 0x0435, "e" }, { 0x0451, "yo" }, { 0x0436, "zh" },
		{ 0x0437, "z" }, { 0x0438, "i" }, { 0x0439, "y" }, { 0x043A, "k" },
		{ 0x043B, "l" }, { 0x043C, "m" }, { 0x043D, "n" }, { 0x043E, "o" },
		{ 0x043F, "p" }, { 0x0440, "r" }, { 0x0441, "s" }, { 0x0442, "t" },
		{ 0x0443, "u" }, { 0x0444, "f" }, { 0x0445, "kh" }, { 0x0446, "ts" },
		{ 0x0447, "ch" }, { 0x0448, "sh" }, { 0x0449, "shch" },
		{ 0x044A, "" }, { 0x044B, "y" }, { 0x044C, "" },
		{ 0x044D, "e" }, { 0x044E, "yu" }, { 0x044F, "ya" }
	};

	char32_t fold_one(char32_t cp) noexcept
	{
		if (cp >= 'A' && cp <= 'Z') return cp + 32;

		if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) return cp + 0x20;
		if (cp == 0x178) return 0xFF;

		if (cp >= 0x391 && cp <= 0x3A9) return cp + 0x20;
		if (cp == 0x3C2) return 0x3C3;

		if (cp >= 0x410 && cp <= 0x42F) return cp + 0x20;
		if (cp == 0x401) return 0x451;

		if (cp >= 0x100 && cp <= 0x137 && cp % 2 == 0) return cp + 1;
		if (cp >= 0x139 && cp <= 0x148 && cp % 2 == 1) return cp + 1;
		if (cp >= 0x14A && cp <= 0x177 && cp % 2 == 0) return cp + 1;
		if (cp == 0x179 || cp == 0x17B || cp == 0x17D) return cp + 1;

		return cp;
	}

	bool is_combining(char32_t cp) noexcept
	{
		return cp >= 0x0300 && cp <= 0x036F;
	}

	// True for uppercase Greek / Cyrillic letters whose romanization comes
	// from LETTERS. Used to keep the first letter's case when folding.
	bool is_upper_letter(char32_t cp) noexcept
	{
		return (cp >= 0x391 && cp <= 0x3A9)
			|| (cp >= 0x386 && cp <= 0x38F)
			|| (cp >= 0x410 && cp <= 0x42F)
			|| cp == 0x401;
	}

	const Decomp* find_by_lower(char32_t lower) noexcept
	{
		for (const auto& d : DECOMP)
		{
			if (d.lower == lower)
				return &d;
		}

		return nullptr;
	}

	const char* find_special(char32_t cp) noexcept
	{
		for (const auto& m : SPECIAL)
		{
			if (m.cp == cp)
				return m.text;
		}

		return nullptr;
	}

	const char* find_letter(char32_t lower) noexcept
	{
		for (const auto& m : LETTERS)
		{
			if (m.cp == lower)
				return m.text;
		}

		return nullptr;
	}

	void append_cp(String& out, char32_t cp)
	{
		if (cp < 0x80)
		{
			out += static_cast<char>(cp);
		}
		else if (cp < 0x800)
		{
			out += static_cast<char>(0xC0 | (cp >> 6));
			out += static_cast<char>(0x80 | (cp & 0x3F));
		}
		else if (cp < 0x10000)
		{
			out += static_cast<char>(0xE0 | (cp >> 12));
			out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (cp & 0x3F));
		}
		else
		{
			out += static_cast<char>(0xF0 | (cp >> 18));
			out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
			out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (cp & 0x3F));
		}
	}

	std::vector<char32_t> points(const sp::cppString& s)
	{
		return sp::utf8::codepoints(s);
	}
}

sp::cppString sp::unicode::fold_case(const cppString& s)
{
	String out;

	for (const char32_t cp : points(s))
		append_cp(out, fold_one(cp));

	return cppString(std::move(out));
}

sp::cppString sp::unicode::to_fullwidth(const cppString& s)
{
	String out;

	for (const char32_t cp : points(s))
	{
		if (cp == 0x20)
			append_cp(out, 0x3000);
		else if (cp >= 0x21 && cp <= 0x7E)
			append_cp(out, cp + 0xFEE0);
		else
			append_cp(out, cp);
	}

	return cppString(std::move(out));
}

sp::cppString sp::unicode::to_halfwidth(const cppString& s)
{
	String out;

	for (const char32_t cp : points(s))
	{
		if (cp == 0x3000)
			append_cp(out, 0x20);
		else if (cp >= 0xFF01 && cp <= 0xFF5E)
			append_cp(out, cp - 0xFEE0);
		else
			append_cp(out, cp);
	}

	return cppString(std::move(out));
}

sp::cppString sp::unicode::transliterate(const cppString& s)
{
	String out;

	for (const char32_t cp : points(s))
	{
		if (const char* special = find_special(cp))
		{
			out += special;
			continue;
		}

		if (cp < 0x80)
		{
			out += static_cast<char>(cp);
			continue;
		}

		const char32_t lower = fold_one(cp);

		if (const Decomp* d = find_by_lower(lower))
		{
			// d->base is uppercase; keep the original letter's case.
			const char base = static_cast<char>(d->base);
			out += (lower == cp) ? static_cast<char>(base - 'A' + 'a') : base;
			continue;
		}

		if (const char* letter = find_letter(lower))
		{
			String mapped{ letter };

			if (is_upper_letter(cp) && !mapped.empty()
				&& mapped.front() >= 'a' && mapped.front() <= 'z')
			{
				mapped.front() = static_cast<char>(mapped.front() - 'a' + 'A');
			}

			out += mapped;
			continue;
		}

		// Combining marks and unmappable code points are dropped.
	}

	return cppString(std::move(out));
}

sp::cppString sp::unicode::decompose(const cppString& s)
{
	String out;

	for (const char32_t cp : points(s))
	{
		if (const Decomp* d = find_by_lower(fold_one(cp)))
		{
			const bool uppercase = (fold_one(cp) != cp);
			const char32_t base = uppercase ? d->base : (d->base - 'A' + 'a');
			append_cp(out, base);
			append_cp(out, d->mark);
		}
		else
		{
			append_cp(out, cp);
		}
	}

	return cppString(std::move(out));
}

sp::cppString sp::unicode::compose(const cppString& s)
{
	const auto cps = points(s);
	String out;

	for (std::size_t i = 0; i < cps.size(); ++i)
	{
		if (i + 1 < cps.size() && is_combining(cps[i + 1]))
		{
			char32_t base = cps[i];

			if (base >= 'a' && base <= 'z')
				base = base - 'a' + 'A';

			bool matched = false;

			for (const auto& d : DECOMP)
			{
				if (d.base != base || d.mark != cps[i + 1])
					continue;

				char32_t composed = d.lower;

				if (cps[i] >= 'A' && cps[i] <= 'Z')
				{
					if (d.lower == 0x00FF)
						composed = 0x0178;       // Y + diaeresis -> capital Y-umlaut
					else if (d.lower >= 0x100)
						composed = d.lower - 1;
					else
						composed = d.lower - 0x20;
				}

				append_cp(out, composed);
				matched = true;
				break;
			}

			if (matched)
			{
				++i;
				continue;
			}
		}

		append_cp(out, cps[i]);
	}

	return cppString(std::move(out));
}
