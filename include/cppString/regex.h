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

#ifndef CPPSTRING_REGEX_H_
#define CPPSTRING_REGEX_H_

#include "cppString.h"
#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

namespace sp
{
	// ECMAScript-flavoured regular expressions with cppString-friendly types.
	// Offsets are byte offsets (UTF-8 safe as long as the pattern itself does
	// not split a multibyte sequence). Invalid patterns throw Invalid_Argument;
	// a replacement that references a non-existent group throws Invalid_Format.
	//
	// Replacement templates understand $0..$9, ${0}..${n}, $&, $$ and \0..\9.
	// All functions are fully qualified as sp::regex:: in the docs on purpose:
	// the name would otherwise collide with std::regex for users doing
	// `using namespace std;` as well.
	namespace regex
	{
		enum class Option : unsigned
		{
			none     = 0,
			icase    = 1u << 0,   // case-insensitive matching
			extended = 1u << 1,   // POSIX ERE grammar instead of ECMAScript
		};

		[[nodiscard]] constexpr Option operator|(Option lhs, Option rhs) noexcept
		{
			return static_cast<Option>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
		}

		[[nodiscard]] constexpr bool has(Option value, Option bit) noexcept
		{
			return (static_cast<unsigned>(value) & static_cast<unsigned>(bit)) != 0;
		}

		struct CPPSTRING_EXPORT Match
		{
			cppString value{};                  // whole match
			std::size_t position{ 0 };          // byte offset of the whole match
			std::size_t length{ 0 };            // byte length of the whole match
			std::vector<cppString> groups{};    // capture groups 1..N

			// index 0 is the whole match; out-of-range throws Index_Out_Of_Range.
			[[nodiscard]] cppString group(std::size_t index) const;
			[[nodiscard]] cppString operator[](std::size_t index) const;
			[[nodiscard]] std::size_t count() const noexcept { return groups.size(); }
		};

		// Escapes regex metacharacters so a string can be used as a literal.
		[[nodiscard]] cppString escape(const cppString& literal);

		[[nodiscard]] bool is_match(const cppString& text, const cppString& pattern,
			Option options = Option::none);

		// First match anywhere in the text.
		[[nodiscard]] std::optional<Match> search(const cppString& text,
			const cppString& pattern, Option options = Option::none);

		// Match anchored at the very beginning of the text (no full-string requirement).
		[[nodiscard]] std::optional<Match> match(const cppString& text,
			const cppString& pattern, Option options = Option::none);

		[[nodiscard]] std::vector<Match> find_all(const cppString& text,
			const cppString& pattern, Option options = Option::none);

		[[nodiscard]] std::size_t count(const cppString& text,
			const cppString& pattern, Option options = Option::none);

		// Replace the first match only.
		[[nodiscard]] cppString replace(const cppString& text, const cppString& pattern,
			const cppString& replacement, Option options = Option::none);

		[[nodiscard]] cppString replace_all(const cppString& text, const cppString& pattern,
			const cppString& replacement, Option options = Option::none);

		// Callback variant: the callback result is used verbatim (no expansion).
		[[nodiscard]] cppString replace_all(const cppString& text, const cppString& pattern,
			const std::function<cppString(const Match&)>& transform,
			Option options = Option::none);

		// Split on the delimiter pattern; trailing empty fields are preserved.
		[[nodiscard]] std::vector<cppString> split(const cppString& text,
			const cppString& pattern, Option options = Option::none);
	}
}

#endif // CPPSTRING_REGEX_H_
