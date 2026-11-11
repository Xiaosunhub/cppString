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

#include "argparse.h"

using sp::ArgParser;
using sp::cppString;

ArgParser::ArgParser(cppString prog, cppString description)
	: m_prog(std::move(prog)), m_description(std::move(description))
{
	add_flag("help", 'h', "Show this help message and exit");
}

void ArgParser::add_flag(const cppString& name, char short_name, const cppString& help)
{
	Entry e;
	e.name = name;
	e.short_name = short_name;
	e.help = help;
	e.is_flag = true;
	m_entries.push_back(std::move(e));
}

void ArgParser::add_option(const cppString& name, char short_name,
	const cppString& help, const cppString& default_value)
{
	Entry e;
	e.name = name;
	e.short_name = short_name;
	e.help = help;
	e.default_value = default_value;
	m_entries.push_back(std::move(e));
}

void ArgParser::add_positional(const cppString& name, const cppString& help)
{
	Entry e;
	e.name = name;
	e.help = help;
	e.is_positional = true;
	m_entries.push_back(std::move(e));
}

const ArgParser::Entry* ArgParser::find_long(const cppString& name) const noexcept
{
	for (const auto& e : m_entries)
	{
		if (!e.is_positional && e.name == name)
			return &e;
	}

	return nullptr;
}

const ArgParser::Entry* ArgParser::find_short(char c) const noexcept
{
	for (const auto& e : m_entries)
	{
		if (!e.is_positional && e.short_name == c)
			return &e;
	}

	return nullptr;
}

void ArgParser::parse(int argc, char** argv)
{
	std::vector<cppString> args;

	for (int i = 1; i < argc; ++i)
		args.emplace_back(argv[i]);

	parse(args);
}

void ArgParser::parse(const std::vector<cppString>& args)
{
	m_values.clear();
	m_positionals.clear();

	bool only_positional = false;

	for (std::size_t i = 0; i < args.size(); ++i)
	{
		const cppString& arg = args[i];

		if (only_positional)
		{
			m_positionals.push_back(arg);
			continue;
		}

		if (arg == "--")
		{
			only_positional = true;
			continue;
		}

		if (arg.startsWith("--"))
		{
			// --name or --name=value
			cppString body = arg.substr(2);
			cppString name = body;
			cppString inline_value;
			bool has_inline = false;

			const auto eq = body.s_str().find('=');

			if (eq != String::npos)
			{
				name = body.substr(0, eq);
				inline_value = body.substr(eq + 1);
				has_inline = true;
			}

			const Entry* e = find_long(name);

			if (e == nullptr)
				throw sp::Invalid_Argument("Unknown option: --" + name.s_str());

			if (e->is_flag)
			{
				if (has_inline)
					throw sp::Invalid_Argument(
						"Flag --" + name.s_str() + " does not take a value");

				m_values[e->name] = "true";
			}
			else
			{
				if (has_inline)
				{
					m_values[e->name] = inline_value;
				}
				else
				{
					if (i + 1 >= args.size())
						throw sp::Invalid_Argument(
							"Option --" + name.s_str() + " needs a value");

					m_values[e->name] = args[++i];
				}
			}

			continue;
		}

		if (arg.size() >= 2 && arg[0] == '-' && arg[1] != '-')
		{
			// -abc: combined flags; an option consumes the rest or the next arg.
			for (std::size_t j = 1; j < arg.size(); ++j)
			{
				const char c = arg[j];
				const Entry* e = find_short(c);

				if (e == nullptr)
					throw sp::Invalid_Argument(String("Unknown option: -") + c);

				if (e->is_flag)
				{
					m_values[e->name] = "true";
					continue;
				}

				// Option: the rest of this arg is the value, or the next arg.
				if (j + 1 < arg.size())
				{
					m_values[e->name] = arg.substr(j + 1);
				}
				else
				{
					if (i + 1 >= args.size())
						throw sp::Invalid_Argument(
							String("Option -") + c + " needs a value");

					m_values[e->name] = args[++i];
				}

				break;
			}

			continue;
		}

		m_positionals.push_back(arg);
	}

	// Bind positionals to their declared names.
	std::size_t declared = 0;

	for (const auto& e : m_entries)
	{
		if (!e.is_positional)
			continue;

		if (declared < m_positionals.size())
			m_values[e.name] = m_positionals[declared];

		++declared;
	}

	if (m_positionals.size() > declared)
		throw sp::Invalid_Argument(
			"Unexpected positional argument: " + m_positionals[declared].s_str());
}

bool ArgParser::has(const cppString& name) const
{
	return m_values.find(name) != m_values.end();
}

cppString ArgParser::get(const cppString& name) const
{
	const auto it = m_values.find(name);

	if (it != m_values.end())
		return it->second;

	for (const auto& e : m_entries)
	{
		if (e.name == name && !e.is_flag && !e.is_positional)
			return e.default_value;
	}

	return cppString{};
}

std::vector<cppString> ArgParser::positionals() const
{
	return m_positionals;
}

cppString ArgParser::help() const
{
	String out;

	if (!m_prog.empty())
	{
		out += "Usage: ";
		out += m_prog.s_str();

		for (const auto& e : m_entries)
		{
			if (e.is_positional)
			{
				out += " <";
				out += e.name.s_str();
				out += ">";
			}
		}

		out += " [options]\n";
	}

	if (!m_description.empty())
	{
		out += m_description.s_str();
		out += "\n";
	}

	if (!out.empty())
		out += "\n";

	// Option column width for alignment.
	std::size_t width = 0;
	std::vector<String> labels;

	for (const auto& e : m_entries)
	{
		String label;

		if (e.is_positional)
		{
			labels.push_back("  " + e.name.s_str());
		}
		else
		{
			String opt = "  ";

			if (e.short_name != '\0')
			{
				opt += '-';
				opt += e.short_name;
				opt += ", ";
			}

			opt += "--";
			opt += e.name.s_str();

			if (!e.is_flag)
			{
				opt += " <";
				opt += e.name.s_str();
				opt += ">";

				if (!e.default_value.empty())
				{
					opt += " (default: ";
					opt += e.default_value.s_str();
					opt += ")";
				}
			}

			labels.push_back(std::move(opt));
		}

		width = (std::max)(width, labels.back().size());
	}

	for (std::size_t i = 0; i < m_entries.size(); ++i)
	{
		out += labels[i];

		if (!m_entries[i].help.empty())
		{
			out.append(width - labels[i].size(), ' ');
			out += "  ";
			out += m_entries[i].help.s_str();
		}

		out += "\n";
	}

	return cppString(std::move(out));
}
