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

#ifndef CPPSTRING_ANSI_H_
#define CPPSTRING_ANSI_H_

#include "cppString.h"
#include <vector>

namespace sp
{
	// Helpers for ANSI/VT100 escape sequences (CSI and OSC). Widths are
	// measured with utf8::display_width, so CJK text keeps its columns
	// while colour codes stay invisible.
	namespace ansi
	{
		enum class Align
		{
			Left,
			Center,
			Right
		};

		// Remove CSI (ESC [ ... final) and OSC (ESC ] ... BEL/ST) sequences.
		cppString strip(const cppString& s);

		// True when the text contains at least one recognised escape sequence.
		bool has_ansi(const cppString& s) noexcept;

		// Display width of the stripped text.
		std::size_t visible_width(const cppString& s);

		// Truncate to max_width display columns without counting escape
		// sequences. When text is cut, a trailing SGR reset is appended so
		// the terminal style does not leak. suffix (if any) is reserved
		// inside max_width and appended verbatim.
		cppString truncate(const cppString& s, std::size_t max_width,
			const cppString& suffix = cppString{});

		// Pad to a visible width; escape sequences are preserved untouched.
		cppString pad(const cppString& s, std::size_t width,
			Align align = Align::Left, char fill = ' ');

		// Wrap text in SGR codes, always closing with a full reset.
		cppString style(const cppString& s, const std::vector<int>& codes);
		cppString fg(const cppString& s, int color256);         // 38;5;n
		cppString rgb(const cppString& s, int r, int g, int b); // 38;2;r;g;b
		cppString bold(const cppString& s);
		cppString underline(const cppString& s);
		cppString inverse(const cppString& s);

		cppString reset();                                       // "\x1b[0m"
	}
}

#endif
