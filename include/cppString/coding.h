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

#ifndef CPPSTRING_CODING_H_
#define CPPSTRING_CODING_H_

#include "cppString.h"

namespace sp
{
	namespace coding
	{
		// Base64: standard alphabet with '=' padding.
		cppString toBase64(const cppString& data);
		cppString fromBase64(const cppString& text);   // throws Invalid_Conversion

		// URL- and filename-safe Base64 (RFC 4648 section 5): alphabet uses
		// '-' and '_' instead of '+' and '/', and no '=' padding is emitted.
		// fromBase64Url also accepts missing padding.
		cppString toBase64Url(const cppString& data);
		cppString fromBase64Url(const cppString& text); // throws Invalid_Conversion

		// Hex: two characters per byte, optional uppercase output.
		cppString toHex(const cppString& data, bool uppercase = false);
		cppString fromHex(const cppString& text);      // throws Invalid_Conversion

		// URL percent-encoding: unreserved characters (A-Z a-z 0-9 - _ . ~) are kept,
		// everything else becomes %XX (uppercase hex).
		cppString urlEncode(const cppString& data);
		cppString urlDecode(const cppString& text);    // accepts %XX and '+', throws Invalid_Conversion

		// Binary: 8 bits per byte, MSB first. fromBinary throws Invalid_Conversion
		// when the length is not a multiple of 8 or contains characters other than 0/1.
		cppString toBinary(const cppString& data);
		cppString fromBinary(const cppString& text);
		bool is_binary(const cppString& text) noexcept;

		// Octal: 3 octal digits per byte. fromOctal throws Invalid_Conversion
		// when the length is not a multiple of 3 or contains digits outside 0-7.
		cppString toOctal(const cppString& data);
		cppString fromOctal(const cppString& text);
		bool is_octal(const cppString& text) noexcept;

		// General integer radix (base 2-36, digits 0-9 then a-z).
		// toBase throws Invalid_Argument when base is out of range.
		// fromBase accepts an optional leading '-' and is case-insensitive;
		// throws Invalid_Conversion on bad digits or overflow.
		cppString toBase(unsigned long long value, unsigned base);
		long long fromBase(const cppString& text, unsigned base);

		// Base32 (RFC 4648 section 6): alphabet A-Z 2-7 with '=' padding.
		// fromBase32 accepts lowercase input and missing padding;
		// throws Invalid_Conversion on characters outside the alphabet.
		cppString toBase32(const cppString& data);
		cppString fromBase32(const cppString& text);

		// Base58 (Bitcoin alphabet, no 0/O/I/l): each leading zero byte
		// becomes one '1'. fromBase58 throws Invalid_Conversion on
		// characters outside the alphabet.
		cppString toBase58(const cppString& data);
		cppString fromBase58(const cppString& text);

		// ---- classical ciphers ----

		// ROT13: self-inverse letter substitution.
		cppString rot13(const cppString& text);

		// Caesar cipher: shifts ASCII letters by shift (any sign, wraps mod 26);
		// every other byte is left untouched.
		cppString caesar(const cppString& text, int shift);

		// ---- Morse code ----

		// Letters and digits become dot/dash groups separated by single spaces,
		// word boundaries become " / ". Throws Invalid_Conversion on characters
		// without a Morse equivalent.
		cppString toMorse(const cppString& text);

		// Inverse of toMorse; accepts single spaces between letters and
		// "/" (optionally surrounded by spaces) between words.
		// Throws Invalid_Conversion on unknown groups.
		cppString fromMorse(const cppString& text);
	}
}

#endif
