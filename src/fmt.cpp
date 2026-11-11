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

#include "fmt.h"
#include "utf8.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace
{
	struct Spec
	{
		char fill = ' ';
		char align = 0;          // 0: strings left, numbers right
		char sign = 0;           // '+', '-' or ' '
		bool alternate = false;
		std::size_t width = 0;
		int precision = -1;
		char type = 0;
	};

	bool is_align(char c) noexcept
	{
		return c == '<' || c == '>' || c == '^';
	}

	bool is_type(char c) noexcept
	{
		return c == 'd' || c == 'x' || c == 'X' || c == 'b' || c == 'o'
			|| c == 'f' || c == 'e' || c == 'g' || c == 's';
	}

	Spec parse_spec(const String& text)
	{
		Spec spec{};
		std::size_t pos = 0;

		// [fill][align] - a fill char is only valid when an align follows it.
		if (pos + 1 < text.size() && is_align(text[pos + 1]) && !is_align(text[pos]))
		{
			spec.fill = text[pos];
			spec.align = text[pos + 1];
			pos += 2;
		}
		else if (pos < text.size() && is_align(text[pos]))
		{
			spec.align = text[pos];
			++pos;
		}

		if (pos < text.size() && (text[pos] == '+' || text[pos] == '-' || text[pos] == ' '))
		{
			spec.sign = text[pos];
			++pos;
		}

		if (pos < text.size() && text[pos] == '#')
		{
			spec.alternate = true;
			++pos;
		}

		// Zero padding is shorthand for fill='0' with right alignment.
		bool zero_pad = false;

		if (pos < text.size() && text[pos] == '0')
		{
			zero_pad = true;
			++pos;
		}

		while (pos < text.size() && text[pos] >= '0' && text[pos] <= '9')
		{
			spec.width = spec.width * 10 + static_cast<std::size_t>(text[pos] - '0');
			++pos;
		}

		if (pos < text.size() && text[pos] == '.')
		{
			++pos;
			spec.precision = 0;

			while (pos < text.size() && text[pos] >= '0' && text[pos] <= '9')
			{
				spec.precision = spec.precision * 10 + (text[pos] - '0');
				++pos;
			}
		}

		if (pos < text.size())
		{
			if (!is_type(text[pos]) || pos + 1 != text.size())
				throw sp::Invalid_Format("Invalid format specifier: " + text);

			spec.type = text[pos];
			++pos;
		}

		if (zero_pad && spec.align == 0)
		{
			spec.fill = '0';
			spec.align = '>';
		}

		return spec;
	}

	String digits_of(unsigned long long value, unsigned base, bool upper)
	{
		const char* alphabet = upper
			? "0123456789ABCDEF"
			: "0123456789abcdef";

		if (value == 0)
			return "0";

		String out;

		while (value != 0)
		{
			out += alphabet[value % base];
			value /= base;
		}

		std::reverse(out.begin(), out.end());
		return out;
	}

	String render_integer(const sp::fmt::detail::Arg& arg, const Spec& spec)
	{
		const bool signed_kind = arg.kind == sp::fmt::detail::Arg::Kind::Signed;
		const bool negative = signed_kind && arg.signed_value < 0;

		const unsigned long long magnitude = negative
			? 0ULL - static_cast<unsigned long long>(arg.signed_value)
			: (signed_kind ? static_cast<unsigned long long>(arg.signed_value)
				: arg.unsigned_value);

		const char type = spec.type == 0 ? 'd' : spec.type;
		unsigned base = 10;
		bool upper = false;
		const char* prefix = "";

		switch (type)
		{
		case 'd': base = 10; break;
		case 'x': base = 16; prefix = spec.alternate ? "0x" : ""; break;
		case 'X': base = 16; upper = true; prefix = spec.alternate ? "0X" : ""; break;
		case 'b': base = 2;  prefix = spec.alternate ? "0b" : ""; break;
		case 'o': base = 8;  prefix = spec.alternate ? "0o" : ""; break;
		default:
			throw sp::Invalid_Argument("Numeric format type incompatible with an integer.");
		}

		String out;

		if (negative)
			out += '-';
		else if (spec.sign == '+')
			out += '+';
		else if (spec.sign == ' ')
			out += ' ';

		out += prefix;
		out += digits_of(magnitude, base, upper);
		return out;
	}

	String render_float(const sp::fmt::detail::Arg& arg, const Spec& spec)
	{
		const char type = spec.type == 0 ? 'g' : spec.type;

		if (type != 'f' && type != 'e' && type != 'g')
			throw sp::Invalid_Argument("String or integer format type incompatible with a float.");

		std::ostringstream stream;
		stream.imbue(std::locale::classic());

		if (type == 'f')
			stream << std::fixed;
		else if (type == 'e')
			stream << std::scientific;

		stream << std::setprecision(spec.precision < 0 ? 6 : spec.precision);

		if (spec.sign == '+')
			stream << std::showpos;

		stream << arg.float_value;
		return stream.str();
	}

	String render_string(const sp::fmt::detail::Arg& arg, const Spec& spec)
	{
		if (spec.type != 0 && spec.type != 's')
			throw sp::Invalid_Argument("Numeric format type incompatible with a string.");

		if (spec.precision < 0)
			return arg.text;

		const auto characters = sp::utf8::chars(sp::cppString{ arg.text });
		const std::size_t limit = static_cast<std::size_t>(spec.precision);
		String out;

		for (std::size_t i = 0; i < limit && i < characters.size(); ++i)
			out += characters[i].s_str();

		return out;
	}

	String apply_width(const String& content, const Spec& spec, bool numeric)
	{
		const std::size_t visible = sp::utf8::display_width(sp::cppString{ content });

		if (visible >= spec.width)
			return content;

		const std::size_t padding = spec.width - visible;
		const char align = spec.align != 0 ? spec.align : (numeric ? '>' : '<');

		std::size_t left = 0;
		std::size_t right = 0;

		if (align == '<')
		{
			right = padding;
		}
		else if (align == '^')
		{
			left = padding / 2;
			right = padding - left;
		}
		else
		{
			left = padding;
		}

		return String(left, spec.fill) + content + String(right, spec.fill);
	}

	String render_arg(const sp::fmt::detail::Arg& arg, const Spec& spec)
	{
		using Kind = sp::fmt::detail::Arg::Kind;

		switch (arg.kind)
		{
		case Kind::String:
		case Kind::Bool:
			return apply_width(render_string(arg, spec), spec, false);

		case Kind::Signed:
		case Kind::Unsigned:
			return apply_width(render_integer(arg, spec), spec, true);

		default:
			return apply_width(render_float(arg, spec), spec, true);
		}
	}
}

namespace sp::fmt::detail
{
	Arg Arg::from_string(String value)
	{
		Arg arg;
		arg.kind = Kind::String;
		arg.text = std::move(value);
		return arg;
	}

	Arg Arg::from_bool(bool value)
	{
		Arg arg;
		arg.kind = Kind::Bool;
		arg.text = value ? "true" : "false";
		return arg;
	}

	Arg Arg::from_signed(long long value)
	{
		Arg arg;
		arg.kind = Kind::Signed;
		arg.signed_value = value;
		return arg;
	}

	Arg Arg::from_unsigned(unsigned long long value)
	{
		Arg arg;
		arg.kind = Kind::Unsigned;
		arg.unsigned_value = value;
		return arg;
	}

	Arg Arg::from_float(double value)
	{
		Arg arg;
		arg.kind = Kind::Float;
		arg.float_value = value;
		return arg;
	}

	cppString format_impl(const cppString& pattern, const std::vector<Arg>& args)
	{
		const String& src = pattern.s_str();
		String out;
		std::size_t auto_index = 0;

		for (std::size_t i = 0; i < src.size();)
		{
			if (src[i] == '{')
			{
				if (i + 1 < src.size() && src[i + 1] == '{')
				{
					out += '{';
					i += 2;
					continue;
				}

				const std::size_t end = src.find('}', i + 1);

				if (end == String::npos)
					throw Invalid_Format("Unmatched '{' in format string.");

				const String field = src.substr(i + 1, end - i - 1);
				const std::size_t colon = field.find(':');
				const String index_part = field.substr(0, colon);
				const String spec_part = colon == String::npos
					? String{} : field.substr(colon + 1);

				std::size_t index = auto_index;

				if (!index_part.empty())
				{
					for (const char c : index_part)
					{
						if (c < '0' || c > '9')
							throw Invalid_Format("Non-numeric argument index in format string.");
					}

					index = 0;

					for (const char c : index_part)
						index = index * 10 + static_cast<std::size_t>(c - '0');
				}
				else
				{
					++auto_index;
				}

				if (index >= args.size())
					throw Index_Out_Of_Range("Format argument index out of range.");

				out += render_arg(args[index], parse_spec(spec_part));
				i = end + 1;
			}
			else if (src[i] == '}')
			{
				if (i + 1 < src.size() && src[i + 1] == '}')
				{
					out += '}';
					i += 2;
					continue;
				}

				throw Invalid_Format("Unmatched '}' in format string.");
			}
			else
			{
				out += src[i];
				++i;
			}
		}

		return cppString(std::move(out));
	}
}
