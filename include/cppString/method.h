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

#ifndef CPPSTRING_METHOD_H_
#define CPPSTRING_METHOD_H_

#include "cppString.h"

namespace sp
{
	void reverse_string(cppString& str);

	void reverse_string_to(cppString& str, cppString& result);
	void reverse_string_to(cppString&& str, cppString& result) noexcept;

	void copy_to(cppString& str, cppString& result);
	void copy_to(cppString&& str, cppString& result) noexcept;

	void move_to(cppString& str, cppString& result);
	void move_to(cppString&& str, cppString& result) noexcept;

	void fill_with(cppString& str, char ch);

	void fill(cppString& str, char ch, std::size_t count);

	void remove_prefix(cppString& str, const cppString& prefix);
	void remove_suffix(cppString& str, const cppString& suffix);

	// ---- text utilities ----

	// Concatenate parts with a separator between them.
	cppString join(const std::vector<cppString>& parts, const cppString& separator);
	cppString join(const std::vector<cppString>& parts, char separator);

	// Split on '\n', "\r\n" or '\r'. A trailing newline does not produce an empty last line.
	std::vector<cppString> split_lines(const cppString& str);

	// Pad to width characters (UTF-8 aware). No-op when already wide enough.
	void pad_left(cppString& str, std::size_t width, char fill = ' ');
	void pad_right(cppString& str, std::size_t width, char fill = ' ');

	// Uppercase the first ASCII letter and lowercase the rest.
	void capitalize(cppString& str);

	// Swap ASCII letter case.
	void swap_case(cppString& str);

	// Replace the first occurrence. Throws Invalid_Argument when from is empty.
	void replace_first(cppString& str, const cppString& from, const cppString& to);

	// ---- file I/O ----

	// Read the whole file as bytes. Throws Invalid_Operation when the file cannot be opened.
	cppString read_file(const String& path);
	std::vector<cppString> read_lines(const String& path);

	// Write (or append) bytes to a file. Throws Invalid_Operation when the file cannot be opened.
	void write_file(const String& path, const cppString& data, bool append = false);

	// ---- random ----

	// Random string of the given length drawn from charset
	// (default: A-Z a-z 0-9). Throws Invalid_Argument when charset is empty.
	cppString generate_random(std::size_t length, const cppString& charset =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

	// ---- case-insensitive conveniences (ASCII only) ----

	bool icontains(const cppString& str, const cppString& sub);
	bool istarts_with(const cppString& str, const cppString& prefix);
	bool iends_with(const cppString& str, const cppString& suffix);

	// ---- splitting ----

	// Split on every occurrence of ANY character in delimiters.
	// Consecutive delimiters produce empty fields; a trailing delimiter produces no trailing empty field.
	std::vector<cppString> split_by_any(const cppString& str, const cppString& delimiters);

	// ---- pattern matching ----

	// Shell-style wildcard match: '*' matches any run (including empty), '?' matches exactly one char.
	// All other characters must match literally.
	bool wildcard_match(const cppString& str, const cppString& pattern);

	// ---- case style conversions ----

	// "HelloWorld" / "hello-world" / "Hello World" -> "hello_world"
	void to_snake_case(cppString& str);
	// "HelloWorld" / "hello_world" / "Hello World" -> "hello-world"
	void to_kebab_case(cppString& str);
	// "hello_world" / "hello-world" / "Hello World" -> "helloWorld"
	void to_camel_case(cppString& str);
	// "hello_world" / "hello-world" / "hello world" -> "HelloWorld"
	void to_pascal_case(cppString& str);

	// ---- JSON ----

	namespace json
	{
		// Escape for embedding in a JSON string literal: " \ \b \f \n \r \t
		// and control characters below 0x20 (as \u00XX). Other bytes pass through unchanged.
		cppString escape(const cppString& text);

		// Inverse of escape. Handles \uXXXX including UTF-16 surrogate pairs.
		// Throws Invalid_Conversion on malformed escape sequences.
		cppString unescape(const cppString& text);
	}

	// ---- CSV ----

	namespace csv
	{
		// Quote and escape a single field per RFC 4180 when it contains , " CR or LF.
		cppString escape(const cppString& field);

		// Parse one CSV line: quoted fields may contain commas and doubled quotes.
		// Throws Invalid_Conversion on an unterminated quote.
		std::vector<cppString> split(const cppString& line);
	}

	// ---- statistics ----

	// Number of whitespace-separated words. A run of CJK characters counts as one word
	// because no space is required between them.
	std::size_t word_count(const cppString& str);

	// Levenshtein edit distance measured in characters (UTF-8 aware).
	std::size_t levenshtein(const cppString& a, const cppString& b);

	// 1.0 for identical strings, 0.0 for completely different ones.
	// Empty vs empty is 1.0; empty vs non-empty is 0.0.
	double similarity(const cppString& a, const cppString& b);

	// Longest common prefix of all parts, cut on a UTF-8 boundary. Empty input -> "".
	cppString longest_common_prefix(const std::vector<cppString>& parts);

	// ---- layout ----

	// Greedy word wrap at width characters (UTF-8 aware). Existing newlines are kept;
	// words longer than the width are left intact on their own line.
	std::vector<cppString> wrap(const cppString& str, std::size_t width);

	// Prefix every non-empty line with prefix.
	void indent(cppString& str, const cppString& prefix);

	// Group the integer part with sep: 1234567 -> "1,234,567". Keeps a leading '-'.
	cppString thousands_sep(long long value, char sep = ',');

	// Human readable byte size: 512 -> "512 B", 1536 -> "1.5 KB", 1048576 -> "1 MB".
	cppString human_size(unsigned long long bytes);

	// Replace every "{{key}}" in tmpl using vars. Unknown placeholders are left untouched.
	cppString render(const cppString& tmpl, const std::map<cppString, cppString>& vars);

	// Split at the first (or last) occurrence of delimiter.
	// When the delimiter is absent the whole string goes to first and second is empty.
	std::pair<cppString, cppString> split_pair(const cppString& str,
		const cppString& delimiter, bool first = true);

	// ---- convenience ----

	// Cut the string to at most max_chars characters (UTF-8 aware), appending
	// ellipsis when anything was removed. The ellipsis is included in max_chars.
	void truncate(cppString& str, std::size_t max_chars, const cppString& ellipsis = "...");

	// URL-friendly slug: lowercase, spaces/underscores -> '-', drop everything
	// outside [a-z0-9-], collapse repeated '-', trim leading/trailing '-'.
	void slug(cppString& str);

	// Split into chunks of size characters (UTF-8 aware; the last chunk may be shorter).
	// Throws Invalid_Argument when size is zero.
	std::vector<cppString> chunk(const cppString& str, std::size_t size);

	// Parse a boolean: "true/1/yes/on/y/t" and "false/0/no/off/n/f", case-insensitive.
	// Throws Invalid_Conversion on anything else.
	bool to_bool(const cppString& str);

	// ---- string distance & diff helpers ----

	// Positional difference count; strings must have equal UTF-8 character
	// counts, otherwise Invalid_Argument is thrown.
	std::size_t hamming(const cppString& a, const cppString& b);

	// Longest shared leading/trailing part (never cuts a UTF-8 sequence).
	cppString common_prefix(const cppString& a, const cppString& b);
	cppString common_suffix(const cppString& a, const cppString& b);

	// ---- formatting ----

	// 90061 -> "1d 1h 1m 1s". Zero becomes "0s".
	cppString human_duration(unsigned long long seconds);

	// Roman numerals for 1..3999 (throws Invalid_Argument outside that range).
	// from_roman accepts uppercase and lowercase, throws Invalid_Conversion
	// on malformed input such as "IIII" or "VX".
	cppString to_roman(int value);
	int from_roman(const cppString& text);

	// ---- HTML ----

	namespace html
	{
		// Escape & < > " ' as &amp; &lt; &gt; &quot; &#39;.
		cppString escape(const cppString& text);

		// Decode named entities (amp lt gt quot apos nbsp) and numeric
		// entities (&#65; / &#x41;) to UTF-8. Throws Invalid_Conversion on
		// malformed or out-of-range entities.
		cppString unescape(const cppString& text);
	}

	// ---- more text tools ----

	// Capitalize the first letter of every word, lowercase the rest.
	void title_case(cppString& str);

	// Pad both sides so the result is width UTF-8 characters wide;
	// the extra character (when odd) goes to the right.
	cppString center(const cppString& str, std::size_t width, char fill = ' ');

	// Replace tabs with spaces, honoring column positions.
	void expand_tabs(cppString& str, std::size_t tab_size = 8);

	// Reverse the order of whitespace-separated words ("one two" -> "two one").
	void reverse_words(cppString& str);

	// UTF-8 character n-grams; throws Invalid_Argument when n is zero.
	std::vector<cppString> ngrams(const cppString& str, std::size_t n);

	// True when the string reads the same forwards and backwards (UTF-8 aware,
	// case-sensitive). Empty strings count as palindromes.
	bool is_palindrome(const cppString& str) noexcept;

	// American Soundex phonetic code (letter + 3 digits, e.g. "Robert" -> "R163").
	// Throws Invalid_Argument when the string has no ASCII letters.
	cppString soundex(const cppString& str);

	// Random RFC 4122 version-4 UUID, e.g. "550e8400-e29b-41d4-a716-446655440000"
	// (always passes validate::is_uuid).
	cppString uuid_v4();

	// ---- more convenience (3.2) ----

	// Remove the longest run of leading whitespace shared by every non-empty
	// line. Useful for embedded multi-line literals.
	void dedent(cppString& str);

	// Collapse runs of the same character into one. With an empty chars set
	// every repeated character is collapsed ("aaab  bb" -> "ab b");
	// otherwise only characters listed in chars are collapsed.
	void squeeze(cppString& str, const cppString& chars = "");

	// Number of non-overlapping occurrences of needle.
	// Throws Invalid_Argument when needle is empty.
	std::size_t count(const cppString& str, const cppString& needle);

	// True when both strings contain the same character multiset.
	// Byte-level comparison: valid UTF-8 inputs compare per character.
	bool is_anagram(const cppString& a, const cppString& b, bool ignore_case = false);

	// Sorensen-Dice bigram similarity, 0.0..1.0 (UTF-8 aware).
	// Two strings shorter than two characters score 1.0 when equal, else 0.0.
	double dice_coefficient(const cppString& a, const cppString& b);

	// ---- diff ----

	// One entry of a line diff: ' ' context, '-' only in old, '+' only in new.
	struct DiffLine
	{
		char tag;
		cppString text;
	};

	// LCS-based line diff. The result replays old_text into new_text:
	// drop every '-' line, keep every ' ' line, insert every '+' line.
	std::vector<DiffLine> diff_lines(const cppString& old_text, const cppString& new_text);

	// Standard unified diff text built from diff_lines(). Returns an empty
	// string when the two texts are identical. context is the number of
	// unchanged lines kept around each change block.
	cppString unified_diff(const cppString& old_text, const cppString& new_text,
		std::size_t context = 3,
		const cppString& old_name = "a", const cppString& new_name = "b");

	// ---- padding / repetition (never truncate) ----

	// Justify to a minimum byte width. center() is available above for
	// centering; these two cover the left/right cases.
	cppString ljust(const cppString& str, std::size_t width, char fill = ' ');
	cppString rjust(const cppString& str, std::size_t width, char fill = ' ');

	// Repeat a string count times; count == 0 yields an empty string.
	cppString repeat(const cppString& str, std::size_t count);
}

#endif
