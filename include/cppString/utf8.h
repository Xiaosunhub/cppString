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

#ifndef CPPSTRING_UTF8_H_
#define CPPSTRING_UTF8_H_

#include "cppString.h"

namespace sp
{
	namespace utf8
	{
		// Strict UTF-8 validation (rejects truncated, overlong, surrogate and > U+10FFFF sequences).
		bool is_valid(const cppString& s);

		// Number of code points (characters), not bytes.
		std::size_t length(const cppString& s);

		// Byte offset of the char_index-th character. Returns cppString::npos when out of range.
		std::size_t offset(const cppString& s, std::size_t char_index);

		// Substring by character index. Throws Empty_String on empty input, Index_Out_Of_Range on bad range.
		cppString sub(const cppString& s, std::size_t start);
		cppString sub(const cppString& s, std::size_t start, std::size_t length);

		// Split into characters.
		std::vector<cppString> chars(const cppString& s);

		// Decode to code points. Throws Invalid_Conversion on malformed input.
		std::vector<char32_t> codepoints(const cppString& s);

		// Encode one code point. Throws Invalid_Argument on surrogate or out-of-range value.
		cppString from_codepoint(char32_t cp);

		// Reverse by character (safe for multi-byte UTF-8).
		cppString reverse(const cppString& s);

		// Terminal display width in columns: East Asian wide/fullwidth
		// characters count as 2, combining marks and control characters
		// as 0, everything else as 1.
		std::size_t display_width(const cppString& s);

		enum class Alignment
		{
			Left,
			Center,
			Right
		};

		// Truncate to at most max_width display columns, never splitting a
		// UTF-8 character. When text is cut, suffix is reserved inside
		// max_width and appended (an ellipsis is the usual choice). If the
		// suffix itself is wider than max_width, it is omitted.
		cppString truncate_by_width(const cppString& s, std::size_t max_width,
			const cppString& suffix = cppString{});

		// Pad to a minimum display width. Returns s unchanged when it is
		// already at least that wide. fill must be a single-byte character.
		cppString pad_by_width(const cppString& s, std::size_t width,
			Alignment align = Alignment::Left, char fill = ' ');

		// UTF-8 byte-order mark (EF BB BF) helpers. strip_bom / add_bom are
		// idempotent and never count the BOM as a character.
		bool has_bom(const cppString& s) noexcept;
		cppString strip_bom(const cppString& s);
		cppString add_bom(const cppString& s);
	}
}

#endif
