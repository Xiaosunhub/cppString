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

#ifndef CPPSTRING_UNICODE_H_
#define CPPSTRING_UNICODE_H_

#include "cppString.h"

namespace sp
{
	// Code-point level Unicode helpers. These are deliberately compact
	// (a curated table for Latin-1, Latin Extended-A, Greek and Cyrillic),
	// not a full UCD implementation. Malformed UTF-8 throws
	// Invalid_Conversion via utf8::codepoints.
	namespace unicode
	{
		// Case folding to lowercase: ASCII, Latin-1, Latin Extended-A,
		// Greek (including final sigma), and Cyrillic.
		[[nodiscard]] cppString fold_case(const cppString& s);

		// ASCII 0x21..0x7E -> U+FF01..U+FF5E, space -> U+3000.
		[[nodiscard]] cppString to_fullwidth(const cppString& s);

		// The inverse of to_fullwidth.
		[[nodiscard]] cppString to_halfwidth(const cppString& s);

		// Best-effort Latin transliteration. Diacritics are dropped,
		// ligatures expanded (AE, OE, ss), and Greek/Cyrillic letters are
		// romanized. Unmappable code points are removed; the result is ASCII.
		[[nodiscard]] cppString transliterate(const cppString& s);

		// NFD-like canonical decomposition of precomposed Latin letters
		// into ASCII base + combining mark (no canonical reordering).
		[[nodiscard]] cppString decompose(const cppString& s);

		// NFC-like recomposition: base letter + combining mark back to a
		// single precomposed code point, where the table has a mapping.
		[[nodiscard]] cppString compose(const cppString& s);
	}
}

#endif
