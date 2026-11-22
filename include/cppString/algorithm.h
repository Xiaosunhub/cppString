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

#ifndef CPPSTRING_ALGORITHM_H_
#define CPPSTRING_ALGORITHM_H_

#include "cppString.h"
#include <vector>

namespace cppstr
{
	// Classic string & sequence algorithms. These complement the plain text
	// helpers in method.h; nothing here duplicates it. Unless a function says
	// otherwise, it treats a cppString as a raw byte sequence, which is the
	// right model for ASCII text and binary payloads alike.
	namespace algorithm
	{
		// ---- substring search ----

		// Knuth-Morris-Pratt: index of the first occurrence of `needle`,
		// or npos. Never quadratic; runs in O(n + m).
		std::size_t kmp_find(const cppString& haystack, const cppString& needle);

		// All occurrences of `needle`, in increasing order. Overlapping
		// matches are included when `overlapping` is true (default).
		std::vector<std::size_t> kmp_find_all(const cppString& haystack,
			const cppString& needle, bool overlapping = true);

		// Rabin-Karp rolling-hash search: index of the first occurrence of
		// `needle`, or npos. Every hash hit is verified byte by byte, so a
		// hash collision can never produce a wrong result.
		std::size_t rabin_karp_find(const cppString& haystack,
			const cppString& needle);

		// ---- permutations ----

		// std-style next permutation on the bytes of `s`. Returns false when
		// `s` was the last permutation, in which case it wraps around to the
		// alphabetically first one.
		bool next_permutation(cppString& s);

		// std-style previous permutation on the bytes of `s`. Returns false
		// when `s` was the first permutation, in which case it wraps around
		// to the alphabetically last one.
		bool prev_permutation(cppString& s);

		// True when `a` and `b` contain exactly the same bytes with the same
		// multiplicities (an anagram test).
		bool is_permutation_of(const cppString& a, const cppString& b) noexcept;

		// ---- common sequence helpers (UTF-8 aware) ----

		// Longest common subsequence of `a` and `b`, compared per Unicode
		// code point. Throws Invalid_Format when either input is not valid
		// UTF-8.
		cppString longest_common_subsequence(const cppString& a,
			const cppString& b);

		// Longest common prefix/suffix shared by every string in the list,
		// compared per Unicode code point. Empty or single-element lists are
		// allowed; an empty result is "".
		cppString longest_common_prefix(const std::vector<cppString>& strings);
		cppString longest_common_suffix(const std::vector<cppString>& strings);

		// Longest run of non-repeating bytes (classic sliding window):
		// "abcabcbb" -> "abc". Byte-level by design so it works on binary
		// data too.
		cppString longest_unique_run(const cppString& s);

		// ---- line-oriented text operations ----

		struct SortLinesOptions
		{
			bool case_insensitive = false; // compare A-Z as a-z (ASCII only)
			bool descending = false;       // reverse the sort order
			bool unique = false;           // drop duplicate lines
		};

		// Sort the '\n'-separated lines of `text`. Lines keep their content
		// (a trailing '\r' stays part of the line); a trailing newline in the
		// input is preserved in the output.
		cppString sort_lines(const cppString& text,
			const SortLinesOptions& options = {});

		// Rotate the bytes of `s` left by `n` positions (n is taken modulo
		// the length): rotate("abcdef", 2) -> "cdefab".
		cppString rotate(const cppString& s, std::size_t n);

		// ---- classic reductions ----

		// Roman numerals in uppercase, 1..3999. Throws Invalid_Argument when
		// `value` is out of range.
		cppString to_roman(unsigned value);

		// Parse a Roman numeral (lenient about non-canonical but letter-valid
		// forms such as "IIII"). Throws Invalid_Format on any character
		// outside MDCLXVI.
		unsigned from_roman(const cppString& numeral);

		// Run-length encoding: "aaabbc" -> "a3b2c1". Every run is written as
		// <char><count> with count >= 1.
		cppString run_length_encode(const cppString& s);

		// Inverse of run_length_encode. Throws Invalid_Format when `s` is not
		// a valid RLE sequence (leading digit, missing or zero count, or
		// trailing garbage).
		cppString run_length_decode(const cppString& s);

		// Boyer-Moore majority vote on bytes: returns true and sets `out`
		// when some byte occurs strictly more than s.size() / 2 times.
		bool majority_byte(const cppString& s, char& out);
	}
}

#endif // CPPSTRING_ALGORITHM_H_
