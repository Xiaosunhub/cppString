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

#ifndef CPPSTRING_FMT_H_
#define CPPSTRING_FMT_H_

#include "cppString.h"
#include <type_traits>
#include <utility>
#include <vector>

namespace sp
{
	// Small, type-safe text formatter. It intentionally does not attempt to
	// be std::format: the grammar is compact and every argument is rendered
	// through the cppString/string primitives of this library.
	//
	//   fmt::format("{} + {} = {}", 1, 2, 3)          -> "1 + 2 = 3"
	//   fmt::format("{1} then {0}", "a", "b")         -> "b then a"
	//   fmt::format("{{literal}}")                    -> "{literal}"
	//   fmt::format("{:>8}", "hi")                    -> "      hi"
	//   fmt::format("{:#x}", 255)                     -> "0xff"
	//   fmt::format("{:.2f}", 3.14159)                -> "3.14"
	//
	// Spec grammar: [fill][<|^|>][+][#][width][.precision][type]
	//   type: d (decimal), x / X (hex), b (binary), o (octal),
	//         f (fixed), e (scientific), g (default), s (string)
	// Bad syntax throws Invalid_Format, an out-of-range index throws
	// Index_Out_Of_Range, and a spec incompatible with the argument type
	// throws Invalid_Argument.
	namespace fmt
	{
		namespace detail
		{
			// Type-erased argument. Conversion is done by the header template;
			// parsing and rendering live in fmt.cpp so there are no template
			// definitions hidden in an implementation file.
			struct CPPSTRING_EXPORT Arg
			{
				enum class Kind { String, Signed, Unsigned, Float, Bool };

				Kind kind = Kind::String;
				String text{};
				long long signed_value = 0;
				unsigned long long unsigned_value = 0;
				double float_value = 0.0;

				static Arg from_string(String value);
				static Arg from_bool(bool value);
				static Arg from_signed(long long value);
				static Arg from_unsigned(unsigned long long value);
				static Arg from_float(double value);
			};

			CPPSTRING_EXPORT cppString format_impl(const cppString& pattern,
				const std::vector<Arg>& args);

			template<typename T>
			Arg make_arg(T&& value)
			{
				using U = std::decay_t<T>;

				if constexpr (std::is_same_v<U, bool>)
				{
					return Arg::from_bool(value);
				}
				else if constexpr (std::is_same_v<U, cppString>)
				{
					return Arg::from_string(value.s_str());
				}
				else if constexpr (std::is_same_v<U, String> || std::is_same_v<U, std::string>)
				{
					return Arg::from_string(String{ value });
				}
				else if constexpr (std::is_same_v<U, const char*> || std::is_same_v<U, char*>)
				{
					return Arg::from_string(value ? String{ value } : String{});
				}
				else if constexpr (std::is_same_v<U, char>)
				{
					return Arg::from_string(String(1, value));
				}
				else if constexpr (std::is_integral_v<U>)
				{
					if constexpr (std::is_signed_v<U>)
						return Arg::from_signed(static_cast<long long>(value));
					else
						return Arg::from_unsigned(static_cast<unsigned long long>(value));
				}
				else if constexpr (std::is_floating_point_v<U>)
				{
					return Arg::from_float(static_cast<double>(value));
				}
				else
				{
					static_assert(sizeof(U) == 0,
						"fmt::format: unsupported argument type (use cppString, a string, a char, or an arithmetic value)");
				}
			}
		}

		template<typename... Args>
		[[nodiscard]] cppString format(const cppString& pattern, Args&&... args)
		{
			std::vector<detail::Arg> erased;
			erased.reserve(sizeof...(Args));
			(erased.push_back(detail::make_arg(std::forward<Args>(args))), ...);
			return detail::format_impl(pattern, erased);
		}
	}
}

#endif
