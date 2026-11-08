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

#ifndef CPPSTRING_INI_H_
#define CPPSTRING_INI_H_

#include "cppString.h"
#include <vector>
#include <utility>
#include <cstddef>

namespace sp
{
	// INI / simple configuration files. Sections and keys preserve the order
	// in which they first appeared. The unnamed (global) section is addressed
	// by an empty name. Comments are parsed but not preserved by dump().
	namespace ini
	{
		class CPPSTRING_EXPORT Document
		{
		public:
			using Entry = std::pair<cppString, cppString>;
			using Section = std::vector<Entry>;

			Document();

			// Named sections only, in file order (the global section is omitted).
			[[nodiscard]] std::vector<cppString> sections() const;
			[[nodiscard]] bool has_section(const cppString& name) const noexcept;

			// Copy of the section's entries. Throws Invalid_Argument when absent.
			[[nodiscard]] Section entries(const cppString& name) const;

			[[nodiscard]] bool has(const cppString& key) const noexcept;
			[[nodiscard]] bool has(const cppString& section, const cppString& key) const noexcept;

			[[nodiscard]] cppString get(const cppString& section, const cppString& key,
				const cppString& fallback = cppString{}) const;

			// Convenience overloads for the unnamed (global) section.
			// They are named explicitly to avoid ambiguity with the
			// (section, key, fallback) form above.
			[[nodiscard]] cppString get_global(const cppString& key,
				const cppString& fallback = cppString{}) const;
			[[nodiscard]] long long get_int_global(const cppString& key,
				long long fallback = 0) const;
			[[nodiscard]] double get_double_global(const cppString& key,
				double fallback = 0.0) const;
			[[nodiscard]] bool get_bool_global(const cppString& key,
				bool fallback = false) const;

			// Typed getters: a missing key returns the fallback, a malformed
			// value throws Invalid_Conversion.
			[[nodiscard]] long long get_int(const cppString& section, const cppString& key,
				long long fallback = 0) const;
			[[nodiscard]] double get_double(const cppString& section, const cppString& key,
				double fallback = 0.0) const;
			[[nodiscard]] bool get_bool(const cppString& section, const cppString& key,
				bool fallback = false) const;

			void set(const cppString& key, const cppString& value);
			void set(const cppString& section, const cppString& key, const cppString& value);

			// Returns false when the key (or section) did not exist.
			bool erase(const cppString& key);
			bool erase(const cppString& section, const cppString& key);
			bool erase_section(const cppString& name);

			[[nodiscard]] cppString dump() const;

		private:
			std::vector<std::pair<cppString, Section>> m_sections{}; // global section is first

			Section* find_section(const cppString& name) noexcept;
			const Section* find_section(const cppString& name) const noexcept;
			Section& make_section(const cppString& name);
		};

		// Accepts ';' and '#' comments, [sections], '=' and ':' separators,
		// quoted values with \n \t \r \\ \" escapes, and duplicate keys
		// (last one wins). Throws Invalid_Format on a line without a separator.
		[[nodiscard]] Document parse(const cppString& text);
	}
}

#endif
