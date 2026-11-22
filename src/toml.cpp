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

#include "toml.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace
{
	using cppstr::cppString;
	using cppstr::json::Type;
	using cppstr::json::Value;
	using object_t = Value::object_t;
	using array_t = Value::array_t;

	constexpr double kMaxExactInteger = 9007199254740991.0; // 2^53 - 1

	bool is_space(char c) noexcept
	{
		return c == ' ' || c == '\t';
	}

	bool is_newline(char c) noexcept
	{
		return c == '\n' || c == '\r';
	}

	bool is_bare_key_char(char c) noexcept
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		       (c >= '0' && c <= '9') || c == '_' || c == '-';
	}

	bool is_dec(char c) noexcept
	{
		return c >= '0' && c <= '9';
	}

	bool is_hex(char c) noexcept
	{
		return is_dec(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	}

	bool is_oct(char c) noexcept
	{
		return c >= '0' && c <= '7';
	}

	bool is_bin(char c) noexcept
	{
		return c == '0' || c == '1';
	}

	int hex_value(char c) noexcept
	{
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		return -1;
	}

	void append_utf8(std::string& out, std::uint32_t cp)
	{
		if (cp <= 0x7F)
		{
			out += static_cast<char>(cp);
		}
		else if (cp <= 0x7FF)
		{
			out += static_cast<char>(0xC0 | (cp >> 6));
			out += static_cast<char>(0x80 | (cp & 0x3F));
		}
		else if (cp <= 0xFFFF)
		{
			out += static_cast<char>(0xE0 | (cp >> 12));
			out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (cp & 0x3F));
		}
		else
		{
			out += static_cast<char>(0xF0 | (cp >> 18));
			out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
			out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (cp & 0x3F));
		}
	}

	object_t::iterator find_key(object_t& obj, const std::string& key)
	{
		return std::find_if(obj.begin(), obj.end(),
			[&key](const auto& kv) { return kv.first.s_str() == key; });
	}

	struct Parser
	{
		const std::string& s;
		std::size_t p = 0;
		std::size_t line = 1;

		explicit Parser(const std::string& text) : s(text) {}

		[[noreturn]] void fail(const std::string& message) const
		{
			throw cppstr::Invalid_Format("toml: " + message + " at line " +
				std::to_string(line) + ", byte " + std::to_string(p));
		}

		bool eof() const noexcept { return p >= s.size(); }
		char peek() const noexcept { return eof() ? '\0' : s[p]; }
		char peek(std::size_t ahead) const noexcept
		{
			return (p + ahead >= s.size()) ? '\0' : s[p + ahead];
		}

		char get()
		{
			if (eof()) fail("unexpected end of document");
			return s[p++];
		}

		bool consume(char c)
		{
			if (peek() == c) { ++p; return true; }
			return false;
		}

		bool starts_with(const char* text) const
		{
			std::size_t n = 0;
			while (text[n] != '\0')
			{
				if (peek(n) != text[n]) return false;
				++n;
			}
			return true;
		}

		void expect(char c)
		{
			if (!consume(c))
			{
				fail(std::string("expected '") + c + "'");
			}
		}

		void skip_space()
		{
			while (!eof() && is_space(peek())) ++p;
		}

		void newline()
		{
			if (consume('\r'))
			{
				consume('\n');
			}
			else
			{
				consume('\n');
			}
			++line;
		}

		void comment()
		{
			// '#' up to (not including) the line ending.
			while (!eof() && !is_newline(peek())) ++p;
		}

		void skip_blank()
		{
			// Whitespace, newlines and comments, used inside arrays.
			while (!eof())
			{
				if (is_space(peek())) { ++p; continue; }
				if (is_newline(peek())) { newline(); continue; }
				if (peek() == '#') { comment(); continue; }
				break;
			}
		}

		void finish_line()
		{
			skip_space();
			if (peek() == '#') comment();
			if (eof()) return;
			if (is_newline(peek())) { newline(); return; }
			fail("unexpected characters after value");
		}

		std::string key()
		{
			if (peek() == '"') return basic_string(false);
			if (peek() == '\'') return literal_string(false);

			const std::size_t begin = p;
			while (!eof() && is_bare_key_char(peek())) ++p;
			if (p == begin) fail("expected a key");
			return s.substr(begin, p - begin);
		}

		std::vector<std::string> key_path()
		{
			std::vector<std::string> parts;
			while (true)
			{
				skip_space();
				parts.push_back(key());
				skip_space();
				if (consume('.')) continue;
				break;
			}
			return parts;
		}

		std::uint32_t hex_digits(std::size_t count)
		{
			std::uint32_t value = 0;
			for (std::size_t i = 0; i < count; ++i)
			{
				if (eof() || hex_value(peek()) < 0)
					fail("incomplete unicode escape");
				value = (value << 4) | static_cast<std::uint32_t>(hex_value(get()));
			}
			return value;
		}

		void escape(std::string& out)
		{
			// The leading backslash has already been consumed.
			if (eof()) fail("dangling escape");
			const char c = get();
			switch (c)
			{
			case 'b': out += '\b'; return;
			case 't': out += '\t'; return;
			case 'n': out += '\n'; return;
			case 'f': out += '\f'; return;
			case 'r': out += '\r'; return;
			case '"': out += '"'; return;
			case '\\': out += '\\'; return;
			case 'u':
			case 'U':
			{
				const std::size_t count = (c == 'u') ? 4 : 8;
				std::uint32_t cp = hex_digits(count);
				if (cp > 0x10FFFF) fail("unicode escape outside the scalar range");
				if (cp >= 0xD800 && cp <= 0xDBFF)
				{
					if (!starts_with("\\u")) fail("high surrogate without a low surrogate");
					p += 2;
					const std::uint32_t low = hex_digits(4);
					if (low < 0xDC00 || low > 0xDFFF)
						fail("invalid low surrogate");
					cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
				}
				else if (cp >= 0xDC00 && cp <= 0xDFFF)
				{
					fail("unpaired low surrogate");
				}
				append_utf8(out, cp);
				return;
			}
			default:
				fail("invalid escape sequence");
			}
		}

		std::string basic_string(bool allow_multiline)
		{
			if (allow_multiline && starts_with("\"\"\""))
				return multiline_basic();

			expect('"');
			std::string out;
			while (true)
			{
				if (eof()) fail("unterminated basic string");
				const char c = get();
				if (c == '"') return out;
				if (is_newline(c)) fail("newline inside a single-line string");
				if (c == '\\') { escape(out); continue; }
				if (static_cast<unsigned char>(c) < 0x20 && c != '\t')
					fail("control character inside a basic string");
				out += c;
			}
		}

		std::string multiline_basic()
		{
			p += 3;
			if (is_newline(peek())) newline(); // trim the first line ending

			std::string out;
			while (true)
			{
				if (eof()) fail("unterminated multiline basic string");
				if (starts_with("\"\"\""))
				{
					p += 3;
					// Up to two extra quotes belong to the string content.
					for (int i = 0; i < 2 && peek() == '"'; ++i)
					{
						out += '"';
						++p;
					}
					return out;
				}

				const char c = get();
				if (c == '\\')
				{
					// A backslash followed by optional spaces and a line ending
					// joins lines: all following whitespace is discarded.
					const std::size_t save = p;
					while (!eof() && is_space(peek())) ++p;
					if (is_newline(peek()))
					{
						newline();
						while (true)
						{
							while (!eof() && is_space(peek())) ++p;
							if (is_newline(peek())) { newline(); continue; }
							break;
						}
						continue;
					}
					p = save;
					escape(out);
					continue;
				}
				if (c == '\r' || c == '\n')
				{
					out += c;
					if (c == '\r' && peek() == '\n') out += get();
					++line;
					continue;
				}
				if (static_cast<unsigned char>(c) < 0x20 && c != '\t')
					fail("control character inside a multiline string");
				out += c;
			}
		}

		std::string literal_string(bool allow_multiline)
		{
			if (allow_multiline && starts_with("'''"))
			{
				p += 3;
				if (is_newline(peek())) newline();
				std::string out;
				while (true)
				{
					if (eof()) fail("unterminated multiline literal string");
					if (starts_with("'''"))
					{
						p += 3;
						for (int i = 0; i < 2 && peek() == '\''; ++i)
						{
							out += '\'';
							++p;
						}
						return out;
					}
					const char c = get();
					if (c == '\r' || c == '\n')
					{
						out += c;
						if (c == '\r' && peek() == '\n') out += get();
						++line;
						continue;
					}
					if (static_cast<unsigned char>(c) < 0x20 && c != '\t')
						fail("control character inside a literal string");
					out += c;
				}
			}

			expect('\'');
			const std::size_t begin = p;
			while (!eof() && peek() != '\'' && !is_newline(peek())) ++p;
			if (eof() || peek() != '\'') fail("unterminated literal string");
			std::string out = s.substr(begin, p - begin);
			++p;
			for (unsigned char c : out)
			{
				if (c < 0x20 && c != '\t')
					fail("control character inside a literal string");
			}
			return out;
		}

		std::string bare_token()
		{
			const std::size_t begin = p;
			while (!eof())
			{
				const char c = peek();
				if (is_space(c) || is_newline(c) || c == ',' || c == ']' ||
				    c == '}' || c == '#') break;
				++p;
			}
			if (p == begin) fail("expected a value");
			return s.substr(begin, p - begin);
		}

		static bool decimal_digit_at(const std::string& t, std::size_t i)
		{
			return i < t.size() && is_dec(t[i]);
		}

		static bool looks_like_datetime(const std::string& t)
		{
			// YYYY-MM-DD (possibly followed by T/t/space and a clock), or HH:MM:SS.
			if (t.size() >= 10 && t[4] == '-' && t[7] == '-' &&
			    decimal_digit_at(t, 0) && decimal_digit_at(t, 1) &&
			    decimal_digit_at(t, 2) && decimal_digit_at(t, 3) &&
			    decimal_digit_at(t, 5) && decimal_digit_at(t, 6) &&
			    decimal_digit_at(t, 8) && decimal_digit_at(t, 9))
				return true;
			return t.find(':') != std::string::npos;
		}

		std::string strip_underscores(const std::string& body, std::size_t begin,
		                              bool (*is_digit)(char))
		{
			std::string out;
			for (std::size_t i = begin; i < body.size(); ++i)
			{
				if (body[i] == '_')
				{
					if (i == begin || i + 1 >= body.size() ||
					    !is_digit(body[i - 1]) || !is_digit(body[i + 1]))
						fail("misplaced underscore in number");
					continue;
				}
				out += body[i];
			}
			return out;
		}

		Value number(const std::string& token)
		{
			if (token.empty()) fail("empty numeric token");

			std::size_t begin = 0;
			bool negative = false;
			if (token[0] == '+' || token[0] == '-')
			{
				negative = (token[0] == '-');
				begin = 1;
			}
			if (begin >= token.size()) fail("sign without digits");

			const std::string rest = token.substr(begin);
			if (rest == "inf")
				return Value(negative ? -std::numeric_limits<double>::infinity()
				                      : std::numeric_limits<double>::infinity());
			if (rest == "nan")
				return Value(std::numeric_limits<double>::quiet_NaN());

			// Prefixed integers: 0x / 0o / 0b. TOML forbids a sign here.
			if (rest.size() > 2 && rest[0] == '0' &&
			    (rest[1] == 'x' || rest[1] == 'o' || rest[1] == 'b'))
			{
				if (begin != 0) fail("prefixed integer cannot carry a sign");
				const char kind = rest[1];
				auto digit_fn = (kind == 'x') ? &is_hex : ((kind == 'o') ? &is_oct : &is_bin);
				const std::string digits = rest.substr(2);
				for (std::size_t i = 0; i < digits.size(); ++i)
				{
					const char c = digits[i];
					if (c != '_' && !digit_fn(c))
						fail("invalid digit in prefixed integer");
				}
				const std::string clean = strip_underscores(digits, 0, digit_fn);
				if (clean.empty()) fail("prefixed integer without digits");
				const int base = (kind == 'x') ? 16 : ((kind == 'o') ? 8 : 2);
				char* end = nullptr;
				const unsigned long long parsed = std::strtoull(clean.c_str(), &end, base);
				if (end != clean.c_str() + clean.size() || parsed > 9007199254740991ULL)
					fail_number_range();
				return Value(static_cast<double>(parsed));
			}

			const bool is_float = rest.find('.') != std::string::npos ||
			                      rest.find('e') != std::string::npos ||
			                      rest.find('E') != std::string::npos;

			// Underscores may only separate decimal digits. The sign is kept
			// apart so the numeric body can be validated independently.
			const std::string body = strip_underscores(rest, 0, &is_dec);
			if (body.empty()) fail("number without digits");
			const std::string signed_body = (negative ? "-" : "") + body;

			if (is_float)
			{
				// digits [ '.' digits ] [ exponent ]; both mantissa sides required.
				std::size_t i = 0;
				const std::size_t int_begin = i;
				while (i < body.size() && is_dec(body[i])) ++i;
				if (i == int_begin) fail("float without an integer part");
				if (i < body.size() && body[i] == '.')
				{
					++i;
					const std::size_t frac_begin = i;
					while (i < body.size() && is_dec(body[i])) ++i;
					if (i == frac_begin) fail("float without a fractional part");
				}
				else if (rest.find('.') != std::string::npos)
				{
					fail("misplaced decimal point");
				}
				if (i < body.size() && (body[i] == 'e' || body[i] == 'E'))
				{
					++i;
					if (i < body.size() && (body[i] == '+' || body[i] == '-')) ++i;
					const std::size_t exp_begin = i;
					while (i < body.size() && is_dec(body[i])) ++i;
					if (i == exp_begin) fail("exponent without digits");
				}
				if (i != body.size()) fail("trailing characters in float");

				char* end = nullptr;
				const double value = std::strtod(signed_body.c_str(), &end);
				if (end != signed_body.c_str() + signed_body.size())
					fail("malformed float");
				if (std::isinf(value)) fail_number_range();
				return Value(value);
			}

			// Decimal integer: no leading zeros (TOML 1.0).
			if (body.size() > 1 && body[0] == '0') fail("leading zero in integer");
			for (char c : body)
			{
				if (!is_dec(c)) fail("invalid decimal integer");
			}
			char* end = nullptr;
			const long long parsed = std::strtoll(signed_body.c_str(), &end, 10);
			if (end != signed_body.c_str() + signed_body.size())
				fail("malformed integer");
			const double magnitude = std::abs(static_cast<double>(parsed));
			if (magnitude > kMaxExactInteger) fail_number_range();
			return Value(static_cast<double>(parsed));
		}

		[[noreturn]] void fail_number_range() const
		{
			throw cppstr::Invalid_Conversion(
				"toml: number is outside the exactly representable double range");
		}

		Value value()
		{
			if (eof()) fail("expected a value");
			const char c = peek();
			if (c == '"') return Value(cppString(basic_string(true)));
			if (c == '\'') return Value(cppString(literal_string(true)));
			if (c == '[') return array();
			if (c == '{') return inline_table();

			const std::string token = bare_token();
			if (token == "true") return Value(true);
			if (token == "false") return Value(false);
			if (looks_like_datetime(token)) return Value(cppString(token));
			return number(token);
		}

		Value array()
		{
			expect('[');
			array_t items;
			while (true)
			{
				skip_blank();
				if (eof()) fail("unterminated array");
				if (consume(']')) break;
				items.push_back(value());
				skip_blank();
				if (consume(','))
				{
					skip_blank();
					if (consume(']')) break; // trailing comma
					continue;
				}
				if (consume(']')) break;
				fail("expected ',' or ']' in array");
			}
			return Value(items);
		}

		Value inline_table()
		{
			expect('{');
			object_t entries;
			while (true)
			{
				skip_space();
				if (eof()) fail("unterminated inline table");
				if (is_newline(peek())) fail("newline inside an inline table");
				if (consume('}')) break;

				auto keys = key_path();
				skip_space();
				expect('=');
				skip_space();
				Value val = value();
				set_in_object(entries, keys, 0, std::move(val));

				skip_space();
				if (consume(','))
				{
					skip_space();
					if (consume('}')) fail("trailing comma in inline table");
					continue;
				}
				if (consume('}')) break;
				fail("expected ',' or '}' in inline table");
			}
			return Value(entries);
		}

		void set_in_object(object_t& obj, const std::vector<std::string>& keys,
		                   std::size_t index, Value val)
		{
			auto it = find_key(obj, keys[index]);
			if (index + 1 == keys.size())
			{
				if (it != obj.end()) fail("duplicate key");
				obj.emplace_back(cppString(keys[index]), std::move(val));
				return;
			}
			if (it == obj.end())
			{
				obj.emplace_back(cppString(keys[index]), Value(object_t{}));
				it = obj.end() - 1;
			}
			if (!it->second.is_object()) fail("dotted key crosses a non-table value");
			set_in_object(it->second.object_items(), keys, index + 1, std::move(val));
		}

		void ensure_table(object_t& obj, const std::vector<std::string>& path,
		                  std::size_t index)
		{
			auto it = find_key(obj, path[index]);
			if (it == obj.end())
			{
				obj.emplace_back(cppString(path[index]), Value(object_t{}));
				it = obj.end() - 1;
			}
			if (!it->second.is_object()) fail("table header conflicts with a value");
			if (index + 1 < path.size())
				ensure_table(it->second.object_items(), path, index + 1);
		}

		static std::string join_path(const std::vector<std::string>& path)
		{
			std::string out;
			for (const auto& part : path)
			{
				out += '\x1f';
				out += part;
			}
			return out;
		}

		Value document()
		{
			Value root(object_t{});
			std::vector<std::string> current;
			std::set<std::string> defined;

			while (true)
			{
				skip_space();
				if (eof()) break;
				if (is_newline(peek())) { newline(); continue; }
				if (peek() == '#') { comment(); continue; }

				if (peek() == '[')
				{
					if (peek(1) == '[') fail("array-of-tables is not supported");
					++p;
					auto header = key_path();
					skip_space();
					expect(']');
					finish_line();
					if (!defined.insert(join_path(header)).second)
						fail("table defined more than once");
					ensure_table(root.object_items(), header, 0);
					current = std::move(header);
					continue;
				}

				auto keys = key_path();
				skip_space();
				expect('=');
				skip_space();
				Value val = value();
				finish_line();

				std::vector<std::string> full = current;
				full.insert(full.end(), keys.begin(), keys.end());
				set_in_object(root.object_items(), full, 0, std::move(val));
			}
			return root;
		}
	};

	std::string quote_basic(const std::string& text);

	std::string key_text(const std::string& key)
	{
		bool bare = !key.empty();
		for (char c : key)
		{
			if (!is_bare_key_char(c)) { bare = false; break; }
		}
		if (bare) return key;
		return quote_basic(key);
	}

	std::string quote_basic(const std::string& text)
	{
		static const char* digits = "0123456789ABCDEF";
		std::string out = "\"";
		for (unsigned char c : text)
		{
			switch (c)
			{
			case '"': out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\b': out += "\\b"; break;
			case '\t': out += "\\t"; break;
			case '\n': out += "\\n"; break;
			case '\f': out += "\\f"; break;
			case '\r': out += "\\r"; break;
			default:
				if (c < 0x20)
				{
					out += "\\u00";
					out += digits[(c >> 4) & 0xF];
					out += digits[c & 0xF];
				}
				else
				{
					out += static_cast<char>(c);
				}
			}
		}
		out += '"';
		return out;
	}

	std::string number_text(double value)
	{
		if (std::isnan(value)) return "nan";
		if (std::isinf(value)) return value < 0 ? "-inf" : "inf";
		if (std::floor(value) == value && std::abs(value) <= kMaxExactInteger)
			return std::to_string(static_cast<long long>(value));

		char buffer[64];
		const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
		std::string out(buffer, result.ptr);
		if (out.find_first_of(".eE") == std::string::npos) out += ".0";
		return out;
	}

	std::string value_text(const Value& val)
	{
		switch (val.type())
		{
		case Type::Null:
			throw cppstr::Invalid_Conversion("toml: null has no TOML representation");
		case Type::Bool:
			return val.as_bool() ? "true" : "false";
		case Type::Number:
			return number_text(val.as_number());
		case Type::String:
			return quote_basic(val.as_string().s_str());
		case Type::Array:
		{
			std::string out = "[";
			bool first = true;
			for (const auto& item : val.as_array())
			{
				if (!first) out += ", ";
				first = false;
				out += value_text(item);
			}
			out += ']';
			return out;
		}
		case Type::Object:
		{
			std::string out = "{";
			bool first = true;
			for (const auto& kv : val.as_object())
			{
				if (!first) out += ", ";
				first = false;
				out += key_text(kv.first.s_str());
				out += " = ";
				out += value_text(kv.second);
			}
			out += '}';
			return out;
		}
		}
		throw cppstr::Invalid_Conversion("toml: unsupported value type");
	}

	// At document level, only a table that contains another table needs its
	// own [section]. Simple root tables stay inline so that key insertion
	// order survives a round trip (json::Value compares objects by order).
	// Inside a section, child tables keep the conventional [a.b] form.
	bool needs_table_section(const Value& val)
	{
		if (!val.is_object()) return false;
		for (const auto& kv : val.as_object())
		{
			if (kv.second.is_object()) return true;
		}
		return false;
	}

	std::string path_text(const std::vector<std::string>& path)
	{
		std::string out;
		for (const auto& part : path)
		{
			if (!out.empty()) out += '.';
			out += key_text(part);
		}
		return out;
	}

	void write_table(const Value& table, const std::vector<std::string>& path,
	                 std::string& out)
	{
		for (const auto& kv : table.as_object())
		{
			if (kv.second.is_object()) continue;
			out += key_text(kv.first.s_str());
			out += " = ";
			out += value_text(kv.second);
			out += '\n';
		}
		for (const auto& kv : table.as_object())
		{
			if (!kv.second.is_object()) continue;
			std::vector<std::string> child = path;
			child.push_back(kv.first.s_str());
			out += "\n[";
			out += path_text(child);
			out += "]\n";
			write_table(kv.second, child, out);
		}
	}
}

namespace cppstr
{
	namespace toml
	{
		json::Value parse(const cppString& text)
		{
			Parser parser(text.s_str());
			return parser.document();
		}

		cppString dump(const json::Value& root)
		{
			if (!root.is_object())
				throw cppstr::Invalid_Conversion("toml::dump requires a table as the root");

			std::string out;
			for (const auto& kv : root.as_object())
			{
				if (needs_table_section(kv.second)) continue;
				out += key_text(kv.first.s_str());
				out += " = ";
				out += value_text(kv.second);
				out += '\n';
			}
			for (const auto& kv : root.as_object())
			{
				if (!needs_table_section(kv.second)) continue;
				std::vector<std::string> path{ kv.first.s_str() };
				out += "\n[";
				out += path_text(path);
				out += "]\n";
				write_table(kv.second, path, out);
			}
			return cppString(out);
		}
	}
}
