// Copyright(c)[2025][Ender_]
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

#include "regex.h"
#include <regex>
#include <string>
#include <utility>

namespace sp
{
	namespace regex
	{
		using Rx = std::regex;
		using RxFlags = std::regex_constants::syntax_option_type;

		namespace
		{
			Rx compile(const cppString& pattern, Option options)
			{
				RxFlags flags = has(options, Option::extended)
					? std::regex_constants::extended
					: std::regex_constants::ECMAScript;
				if (has(options, Option::icase))
					flags = flags | std::regex_constants::icase;

				try
				{
					return Rx{ pattern.s_str(), flags };
				}
				catch (const std::regex_error& e)
				{
					throw sp::Invalid_Argument(String{ "sp::regex: invalid pattern: " } + e.what());
				}
			}

			Match to_match(const std::smatch& m, const String& s)
			{
				Match item{};
				item.value = cppString{ m[0].str() };
				item.position = static_cast<std::size_t>(m[0].first - s.cbegin());
				item.length = static_cast<std::size_t>(m[0].second - m[0].first);
				item.groups.reserve(m.size() > 0 ? m.size() - 1 : 0);
				for (std::size_t g = 1; g < m.size(); ++g)
					item.groups.push_back(cppString{ m[g].matched ? m[g].str() : String{} });
				return item;
			}

			cppString expand(const cppString& replacement, const Match& m)
			{
				const String& r = replacement.s_str();
				String out{};
				out.reserve(r.size());

				auto append_group = [&](std::size_t index)
				{
					if (index > m.count())
						throw sp::Invalid_Format("sp::regex: replacement references a missing capture group");
					out += m.group(index).s_str();
				};

				for (std::size_t i = 0; i < r.size();)
				{
					const char c = r[i];

					if (c == '$' && i + 1 < r.size())
					{
						const char n = r[i + 1];
						if (n == '$') { out += '$'; i += 2; continue; }
						if (n == '&') { out += m.value.s_str(); i += 2; continue; }

						if (n == '{')
						{
							const std::size_t close = r.find('}', i + 2);
							if (close != String::npos && close > i + 2 && close - i - 2 <= 3)
							{
								const String digits = r.substr(i + 2, close - i - 2);
								bool numeric = true;
								for (const char d : digits)
									if (d < '0' || d > '9') { numeric = false; break; }

								if (numeric)
								{
									append_group(static_cast<std::size_t>(std::stoul(digits)));
									i = close + 1;
									continue;
								}
							}
						}
						else if (n >= '0' && n <= '9')
						{
							append_group(static_cast<std::size_t>(n - '0'));
							i += 2;
							continue;
						}

						out += c;   // unknown '$' sequence is literal
						++i;
						continue;
					}

					if (c == '\\' && i + 1 < r.size() && r[i + 1] >= '0' && r[i + 1] <= '9')
					{
						append_group(static_cast<std::size_t>(r[i + 1] - '0'));
						i += 2;
						continue;
					}

					out += c;
					++i;
				}

				return cppString{ std::move(out) };
			}
		}

		cppString Match::group(std::size_t index) const
		{
			if (index == 0)
				return value;
			if (index > groups.size())
				throw sp::Index_Out_Of_Range("sp::regex::Match::group: index out of range");
			return groups[index - 1];
		}

		cppString Match::operator[](std::size_t index) const
		{
			return group(index);
		}

		cppString escape(const cppString& literal)
		{
			static const String meta = R"(\.^$|()[]{}*+?)";
			String out{};
			out.reserve(literal.size() * 2);
			for (const char c : literal.s_str())
			{
				if (meta.find(c) != String::npos)
					out += '\\';
				out += c;
			}
			return cppString{ std::move(out) };
		}

		bool is_match(const cppString& text, const cppString& pattern, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			return std::regex_search(s.cbegin(), s.cend(), re);
		}

		std::optional<Match> search(const cppString& text, const cppString& pattern, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			std::smatch m{};
			if (!std::regex_search(s.cbegin(), s.cend(), m, re))
				return std::nullopt;
			return to_match(m, s);
		}

		std::optional<Match> match(const cppString& text, const cppString& pattern, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			std::smatch m{};
			if (!std::regex_search(s.cbegin(), s.cend(), m, re,
				std::regex_constants::match_continuous))
				return std::nullopt;
			return to_match(m, s);
		}

		std::vector<Match> find_all(const cppString& text, const cppString& pattern, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			std::vector<Match> out{};
			for (auto it = std::sregex_iterator(s.cbegin(), s.cend(), re);
				it != std::sregex_iterator(); ++it)
				out.push_back(to_match(*it, s));
			return out;
		}

		std::size_t count(const cppString& text, const cppString& pattern, Option options)
		{
			return find_all(text, pattern, options).size();
		}

		cppString replace(const cppString& text, const cppString& pattern,
			const cppString& replacement, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			std::smatch m{};
			if (!std::regex_search(s.cbegin(), s.cend(), m, re))
				return text;

			const Match one = to_match(m, s);
			String out = s.substr(0, one.position);
			out += expand(replacement, one).s_str();
			out += s.substr(one.position + one.length);
			return cppString{ std::move(out) };
		}

		cppString replace_all(const cppString& text, const cppString& pattern,
			const cppString& replacement, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			String out{};
			out.reserve(s.size());
			std::size_t last = 0;
			bool matched = false;

			for (auto it = std::sregex_iterator(s.cbegin(), s.cend(), re);
				it != std::sregex_iterator(); ++it)
			{
				const Match one = to_match(*it, s);
				matched = true;
				if (one.position >= last)
					out += s.substr(last, one.position - last);
				out += expand(replacement, one).s_str();
				if (one.position + one.length > last)
					last = one.position + one.length;
			}

			if (!matched)
				return text;
			out += s.substr(last);
			return cppString{ std::move(out) };
		}

		cppString replace_all(const cppString& text, const cppString& pattern,
			const std::function<cppString(const Match&)>& transform, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			String out{};
			out.reserve(s.size());
			std::size_t last = 0;
			bool matched = false;

			for (auto it = std::sregex_iterator(s.cbegin(), s.cend(), re);
				it != std::sregex_iterator(); ++it)
			{
				const Match one = to_match(*it, s);
				matched = true;
				if (one.position >= last)
					out += s.substr(last, one.position - last);
				out += transform(one).s_str();
				if (one.position + one.length > last)
					last = one.position + one.length;
			}

			if (!matched)
				return text;
			out += s.substr(last);
			return cppString{ std::move(out) };
		}

		std::vector<cppString> split(const cppString& text, const cppString& pattern, Option options)
		{
			const Rx re = compile(pattern, options);
			const String& s = text.s_str();
			std::vector<cppString> out{};
			std::size_t last = 0;

			for (auto it = std::sregex_iterator(s.cbegin(), s.cend(), re);
				it != std::sregex_iterator(); ++it)
			{
				const Match one = to_match(*it, s);
				if (one.length == 0 || one.position < last)
					continue;
				out.push_back(cppString{ s.substr(last, one.position - last) });
				last = one.position + one.length;
			}

			out.push_back(cppString{ s.substr(last) });
			return out;
		}
	}
}
