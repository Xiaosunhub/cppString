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

#ifndef CPPSTRING_ARGPARSE_H_
#define CPPSTRING_ARGPARSE_H_

#include "cppString.h"

namespace sp
{
	// Small command-line parser: long options (--name / --name=value),
	// short options (-n), combined short flags (-abc), "--" terminator
	// and positionals. A -h/--help flag is always available.
	//
	// parse() throws Invalid_Argument on unknown options, missing values
	// or surplus positionals.
	class CPPSTRING_EXPORT ArgParser
	{
	public:
		explicit ArgParser(cppString prog = "", cppString description = "");

		// Boolean flag: present or not.
		void add_flag(const cppString& name, char short_name, const cppString& help);

		// Option taking a value: "--output file" or "--output=file" or "-o file".
		void add_option(const cppString& name, char short_name,
			const cppString& help, const cppString& default_value = "");

		// Positional argument, consumed in declaration order.
		void add_positional(const cppString& name, const cppString& help);

		// Parse argv. argv[0] is skipped. Throws Invalid_Argument on error.
		void parse(int argc, char** argv);
		void parse(const std::vector<cppString>& args);

		// True when a flag was given or an option was set explicitly.
		[[nodiscard]] bool has(const cppString& name) const;

		// Option value, its default, or "" when unknown. Positional values
		// are retrievable by their name as well.
		[[nodiscard]] cppString get(const cppString& name) const;

		// All positional values in order.
		[[nodiscard]] std::vector<cppString> positionals() const;

		// Auto-generated help text (usage line + option table).
		[[nodiscard]] cppString help() const;

	private:
		struct Entry
		{
			cppString name;
			char short_name = '\0';
			cppString help;
			cppString default_value;
			bool is_flag = false;
			bool is_positional = false;
		};

		cppString m_prog;
		cppString m_description;
		std::vector<Entry> m_entries;
		std::map<cppString, cppString> m_values;   // name -> value given
		std::vector<cppString> m_positionals;

		const Entry* find_long(const cppString& name) const noexcept;
		const Entry* find_short(char c) const noexcept;
	};
}

#endif
