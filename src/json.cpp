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

#include "json.h"
#include "method.h"
#include "utf8.h"
#include <charconv>
#include <cmath>

using sp::cppString;
using sp::json::Type;
using sp::json::Value;

namespace
{
	// static_cast<double>(LLONG_MAX) rounds UP to 2^63, so an inclusive
	// "value <= double(LLONG_MAX)" check lets 2^63 through and the subsequent
	// cast to long long wraps to LLONG_MIN. The upper bound must be exclusive.
	constexpr double kIntLo = -9223372036854775808.0;  // -2^63, exact
	constexpr double kIntHi = 9223372036854775808.0;   //  2^63, exclusive

	bool fits_long_long(double value) noexcept
	{
		return value >= kIntLo && value < kIntHi && std::trunc(value) == value;
	}
}

Value::Value() = default;

Value::Value(std::nullptr_t) {}

Value::Value(bool value)
	: m_type(Type::Bool), m_bool(value) {}

Value::Value(const char* value)
	: m_type(Type::String), m_string(value) {}

Value::Value(const cppString& value)
	: m_type(Type::String), m_string(value) {}

Value::Value(const array_t& value)
	: m_type(Type::Array), m_array(value) {}

Value::Value(const object_t& value)
	: m_type(Type::Object), m_object(value) {}

bool Value::is_integer() const noexcept
{
	return m_type == Type::Number && fits_long_long(m_number);
}

bool Value::as_bool() const
{
	if (m_type != Type::Bool)
		throw sp::Invalid_Operation("json::Value is not a bool.");

	return m_bool;
}

double Value::as_number() const
{
	if (m_type != Type::Number)
		throw sp::Invalid_Operation("json::Value is not a number.");

	return m_number;
}

long long Value::as_int() const
{
	if (!is_integer())
		throw sp::Invalid_Operation("json::Value is not an integer.");

	return static_cast<long long>(m_number);
}

const cppString& Value::as_string() const
{
	if (m_type != Type::String)
		throw sp::Invalid_Operation("json::Value is not a string.");

	return m_string;
}

const Value::array_t& Value::as_array() const
{
	if (m_type != Type::Array)
		throw sp::Invalid_Operation("json::Value is not an array.");

	return m_array;
}

const Value::object_t& Value::as_object() const
{
	if (m_type != Type::Object)
		throw sp::Invalid_Operation("json::Value is not an object.");

	return m_object;
}

Value::array_t& Value::array_items()
{
	if (m_type == Type::Null)
	{
		m_type = Type::Array;
		m_array.clear();
	}

	if (m_type != Type::Array)
		throw sp::Invalid_Operation("json::Value is not an array.");

	return m_array;
}

Value::object_t& Value::object_items()
{
	if (m_type == Type::Null)
	{
		m_type = Type::Object;
		m_object.clear();
	}

	if (m_type != Type::Object)
		throw sp::Invalid_Operation("json::Value is not an object.");

	return m_object;
}

const Value& Value::operator[](std::size_t index) const
{
	if (m_type != Type::Array)
		throw sp::Invalid_Operation("json::Value is not an array.");

	if (index >= m_array.size())
		throw sp::Index_Out_Of_Range("json::Value array index out of range.");

	return m_array[index];
}

const Value& Value::operator[](const cppString& key) const
{
	static const Value NULL_VALUE{};

	if (m_type != Type::Object)
		throw sp::Invalid_Operation("json::Value is not an object.");

	for (const auto& [k, v] : m_object)
	{
		if (k == key)
			return v;
	}

	return NULL_VALUE;
}

bool Value::contains(const cppString& key) const noexcept
{
	if (m_type != Type::Object)
		return false;

	for (const auto& [k, v] : m_object)
	{
		if (k == key)
			return true;
	}

	return false;
}

std::size_t Value::size() const
{
	switch (m_type)
	{
	case Type::Array:  return m_array.size();
	case Type::Object: return m_object.size();
	case Type::String: return sp::utf8::length(m_string);
	default:           return 0;
	}
}

bool Value::operator==(const Value& other) const
{
	if (m_type != other.m_type)
		return false;

	switch (m_type)
	{
	case Type::Null:   return true;
	case Type::Bool:   return m_bool == other.m_bool;
	case Type::Number: return m_number == other.m_number;
	case Type::String: return m_string == other.m_string;
	case Type::Array:  return m_array == other.m_array;
	case Type::Object: return m_object == other.m_object;
	}

	return false;
}

namespace
{
	cppString number_to_string(double value)
	{
		// RFC 8259 has no NaN/Infinity literal. std::to_chars would emit
		// "nan"/"inf", i.e. text that json::parse can never read back, so
		// reject non-finite values instead of silently producing bad JSON.
		if (!std::isfinite(value))
			throw sp::Invalid_Conversion("Cannot serialize non-finite JSON number (NaN/Infinity).");

		// Integers print without a decimal point.
		if (fits_long_long(value))
			return cppString{ std::to_string(static_cast<long long>(value)) };

		char buf[64];
		const auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value);

		if (ec != std::errc())
			throw sp::Invalid_Conversion("Cannot serialize JSON number.");

		return cppString{ String(buf, static_cast<std::size_t>(ptr - buf)) };
	}

	void dump_into(const Value& v, int indent, int depth, String& out)
	{
		auto pad = [&](int d)
		{
			if (indent >= 0)
				out.append(static_cast<std::size_t>(d * indent), ' ');
		};

		switch (v.type())
		{
		case Type::Null:
			out += "null";
			return;
		case Type::Bool:
			out += v.as_bool() ? "true" : "false";
			return;
		case Type::Number:
			out += number_to_string(v.as_number()).s_str();
			return;
		case Type::String:
			out += '"';
			out += sp::json::escape(v.as_string()).s_str();
			out += '"';
			return;
		case Type::Array:
		{
			const auto& items = v.as_array();

			if (items.empty())
			{
				out += "[]";
				return;
			}

			out += '[';

			for (std::size_t i = 0; i < items.size(); ++i)
			{
				if (i > 0)
					out += ',';

				if (indent >= 0)
				{
					out += '\n';
					pad(depth + 1);
				}

				dump_into(items[i], indent, depth + 1, out);
			}

			if (indent >= 0)
			{
				out += '\n';
				pad(depth);
			}

			out += ']';
			return;
		}
		case Type::Object:
		{
			const auto& items = v.as_object();

			if (items.empty())
			{
				out += "{}";
				return;
			}

			out += '{';

			for (std::size_t i = 0; i < items.size(); ++i)
			{
				if (i > 0)
					out += ',';

				if (indent >= 0)
				{
					out += '\n';
					pad(depth + 1);
				}

				out += '"';
				out += sp::json::escape(items[i].first).s_str();
				out += "\":";
				out += indent >= 0 ? " " : "";

				dump_into(items[i].second, indent, depth + 1, out);
			}

			if (indent >= 0)
			{
				out += '\n';
				pad(depth);
			}

			out += '}';
			return;
		}
		}
	}
}

cppString Value::dump(int indent) const
{
	String out;
	dump_into(*this, indent, 0, out);
	return cppString(std::move(out));
}

namespace
{
	// Recursive-descent parser over the raw bytes.
	class Parser
	{
	public:
		explicit Parser(const String& src) : m_src(src) {}

		Value run()
		{
			skip_ws();
			Value v = parse_value();
			skip_ws();

			if (m_pos != m_src.size())
				fail("Trailing characters after the JSON document");

			return v;
		}

	private:
		const String& m_src;
		std::size_t m_pos = 0;

		[[noreturn]] void fail(const String& what) const
		{
			throw sp::Invalid_Conversion(
				"JSON parse error at byte " + std::to_string(m_pos) + ": " + what);
		}

		void skip_ws() noexcept
		{
			while (m_pos < m_src.size()
				&& (m_src[m_pos] == ' ' || m_src[m_pos] == '\t'
					|| m_src[m_pos] == '\n' || m_src[m_pos] == '\r'))
			{
				++m_pos;
			}
		}

		char peek() const noexcept
		{
			return m_pos < m_src.size() ? m_src[m_pos] : '\0';
		}

		bool consume(char c) noexcept
		{
			if (peek() == c)
			{
				++m_pos;
				return true;
			}

			return false;
		}

		void expect(char c)
		{
			if (!consume(c))
				fail(String("Expected '") + c + "'");
		}

		void expect_literal(const char* word)
		{
			for (const char* p = word; *p; ++p)
			{
				if (m_pos >= m_src.size() || m_src[m_pos] != *p)
					fail(String("Invalid literal, expected '") + word + "'");

				++m_pos;
			}
		}

		Value parse_value()
		{
			switch (peek())
			{
			case 'n': expect_literal("null");  return Value{ nullptr };
			case 't': expect_literal("true");  return Value{ true };
			case 'f': expect_literal("false"); return Value{ false };
			case '"': return Value{ cppString(parse_string()) };
			case '[': return parse_array();
			case '{': return parse_object();
			default:
				if (peek() == '-' || (peek() >= '0' && peek() <= '9'))
					return parse_number();

				fail(m_pos < m_src.size() ? "Unexpected character" : "Unexpected end of input");
			}
		}

		Value parse_number()
		{
			const std::size_t start = m_pos;

			consume('-');

			if (consume('0'))
			{
				if (peek() >= '0' && peek() <= '9')
					fail("Leading zeros are not allowed in numbers");
			}
			else if (peek() >= '1' && peek() <= '9')
			{
				while (peek() >= '0' && peek() <= '9')
					++m_pos;
			}
			else
			{
				fail("Invalid number");
			}

			if (consume('.'))
			{
				if (!(peek() >= '0' && peek() <= '9'))
					fail("Fraction needs at least one digit");

				while (peek() >= '0' && peek() <= '9')
					++m_pos;
			}

			if (peek() == 'e' || peek() == 'E')
			{
				++m_pos;

				if (peek() == '+' || peek() == '-')
					++m_pos;

				if (!(peek() >= '0' && peek() <= '9'))
					fail("Exponent needs at least one digit");

				while (peek() >= '0' && peek() <= '9')
					++m_pos;
			}

			const String text = m_src.substr(start, m_pos - start);
			double value = 0.0;
			const auto [ptr, ec] = std::from_chars(
				text.data(), text.data() + text.size(), value);

			if (ec != std::errc() || ptr != text.data() + text.size())
				fail("Number out of range");

			return Value{ value };
		}

		String parse_string()
		{
			expect('"');
			String raw;

			while (true)
			{
				if (m_pos >= m_src.size())
					fail("Unterminated string");

				const char c = m_src[m_pos];

				if (c == '"')
				{
					++m_pos;
					break;
				}

				if (c == '\\')
				{
					// Keep the escape verbatim; json::unescape decodes it,
					// including \uXXXX surrogate pairs.
					raw += c;
					++m_pos;

					if (m_pos >= m_src.size())
						fail("Unterminated escape sequence");

					raw += m_src[m_pos];
					++m_pos;

					if (raw.back() == 'u')
					{
						for (int i = 0; i < 4; ++i)
						{
							if (m_pos >= m_src.size())
								fail("Unterminated \\u escape");

							raw += m_src[m_pos];
							++m_pos;
						}
					}

					continue;
				}

				if (static_cast<unsigned char>(c) < 0x20)
					fail("Unescaped control character in string");

				raw += c;
				++m_pos;
			}

			return sp::json::unescape(cppString(std::move(raw))).s_str();
		}

		Value parse_array()
		{
			expect('[');
			skip_ws();

			Value::array_t items;

			if (consume(']'))
				return Value{ items };

			while (true)
			{
				skip_ws();
				items.push_back(parse_value());
				skip_ws();

				if (consume(']'))
					break;

				expect(',');
			}

			return Value{ items };
		}

		Value parse_object()
		{
			expect('{');
			skip_ws();

			Value::object_t items;

			if (consume('}'))
				return Value{ items };

			while (true)
			{
				skip_ws();

				if (peek() != '"')
					fail("Object keys must be strings");

				cppString key{ parse_string() };
				skip_ws();
				expect(':');
				skip_ws();
				Value parsed = parse_value();

				// RFC 8259: names SHOULD be unique. Keep one entry per key,
				// with the last occurrence winning (the position of the first
				// occurrence is preserved), as mainstream JSON readers do.
				bool replaced = false;
				for (auto& entry : items)
				{
					if (entry.first == key)
					{
						entry.second = std::move(parsed);
						replaced = true;
						break;
					}
				}
				if (!replaced)
					items.emplace_back(std::move(key), std::move(parsed));

				skip_ws();

				if (consume('}'))
					break;

				expect(',');
			}

			return Value{ items };
		}
	};
}

Value sp::json::parse(const cppString& text)
{
	return Parser(text.s_str()).run();
}

// ---- JSON Pointer (RFC 6901) and Merge Patch (RFC 7386) ----

namespace
{
	std::vector<String> pointer_tokens(const String& path)
	{
		std::vector<String> tokens;

		if (path.empty())
			return tokens;

		if (path.front() != '/')
			throw sp::Invalid_Conversion("JSON Pointer must be empty or start with '/'.");

		std::size_t pos = 0;

		while (pos < path.size())
		{
			++pos; // skip the leading '/'
			const std::size_t end = path.find('/', pos);
			const String raw = path.substr(pos,
				end == String::npos ? String::npos : end - pos);

			String token;

			for (std::size_t i = 0; i < raw.size(); ++i)
			{
				if (raw[i] == '~')
				{
					// RFC 6901: '~' may only introduce the escapes ~0 and ~1.
					// A trailing '~' or '~2' is malformed, not a literal '~'.
					if (i + 1 >= raw.size()
						|| (raw[i + 1] != '0' && raw[i + 1] != '1'))
					{
						throw sp::Invalid_Conversion(
							"Invalid JSON Pointer escape sequence: " + raw);
					}

					token += (raw[i + 1] == '0') ? '~' : '/';
					++i;
				}
				else
				{
					token += raw[i];
				}
			}

			tokens.push_back(std::move(token));

			if (end == String::npos)
				break;

			pos = end;
		}

		return tokens;
	}

	bool parse_index(const String& token, std::size_t& index)
	{
		if (token.empty())
			return false;

		// RFC 6901: array-index = "0" / (1-9 *DIGIT); leading zeros are
		// forbidden and must not silently resolve to a different element.
		if (token.size() > 1 && token.front() == '0')
			return false;

		for (const char c : token)
		{
			if (c < '0' || c > '9')
				return false;
		}

		std::size_t value = 0;
		const auto result = std::from_chars(token.data(), token.data() + token.size(), value);

		if (result.ec != std::errc{} || result.ptr != token.data() + token.size())
			return false;

		index = value;
		return true;
	}
}

const sp::json::Value& sp::json::pointer(const Value& root, const cppString& path)
{
	const auto tokens = pointer_tokens(path.s_str());
	const Value* current = &root;

	for (const String& token : tokens)
	{
		if (current->is_array())
		{
			std::size_t index = 0;

			if (!parse_index(token, index))
				throw sp::Invalid_Conversion("Non-numeric JSON Pointer array token: " + token);

			if (index >= current->as_array().size())
				throw sp::Index_Out_Of_Range("JSON Pointer array index out of range.");

			current = &(*current)[index];
		}
		else if (current->is_object())
		{
			const Value* found = nullptr;

			for (const auto& entry : current->as_object())
			{
				if (entry.first == cppString{ token })
				{
					found = &entry.second;
					break;
				}
			}

			if (!found)
				throw sp::Invalid_Operation("JSON Pointer key not found: " + token);

			current = found;
		}
		else
		{
			throw sp::Invalid_Operation("JSON Pointer traverses a scalar value.");
		}
	}

	return *current;
}

void sp::json::set_pointer(Value& root, const cppString& path, Value value)
{
	const auto tokens = pointer_tokens(path.s_str());

	if (tokens.empty())
	{
		root = std::move(value);
		return;
	}

	Value* current = &root;

	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		const String& token = tokens[i];
		const bool last = (i + 1 == tokens.size());
		std::size_t dummy = 0;

		const bool treat_as_array = current->is_array()
			|| (current->is_null() && (token == "-" || parse_index(token, dummy)));

		if (treat_as_array)
		{
			auto& array = current->array_items();

			if (token == "-")
			{
				if (last)
				{
					array.push_back(std::move(value));
					return;
				}

				array.emplace_back();
				current = &array.back();
				continue;
			}

			std::size_t index = 0;

			if (!parse_index(token, index))
				throw sp::Invalid_Conversion("Non-numeric JSON Pointer array token: " + token);

			while (array.size() <= index)
				array.emplace_back();

			if (last)
			{
				array[index] = std::move(value);
				return;
			}

			current = &array[index];
		}
		else
		{
			auto& object = current->object_items();
			std::size_t found = object.size();

			for (std::size_t k = 0; k < object.size(); ++k)
			{
				if (object[k].first == cppString{ token })
				{
					found = k;
					break;
				}
			}

			if (found == object.size())
			{
				object.emplace_back(cppString{ token }, Value{});
				found = object.size() - 1;
			}

			if (last)
			{
				object[found].second = std::move(value);
				return;
			}

			current = &object[found].second;
		}
	}
}

sp::json::Value sp::json::merge(Value target, const Value& patch)
{
	if (!patch.is_object())
		return patch;

	if (!target.is_object())
	{
		Value empty_object;
		empty_object.object_items();
		target = std::move(empty_object);
	}

	auto& object = target.object_items();

	for (const auto& entry : patch.as_object())
	{
		std::size_t found = object.size();

		for (std::size_t k = 0; k < object.size(); ++k)
		{
			if (object[k].first == entry.first)
			{
				found = k;
				break;
			}
		}

		if (entry.second.is_null())
		{
			if (found != object.size())
				object.erase(object.begin() + static_cast<std::ptrdiff_t>(found));
		}
		else if (found != object.size())
		{
			object[found].second = merge(std::move(object[found].second), entry.second);
		}
		else
		{
			object.emplace_back(entry.first, merge(Value{}, entry.second));
		}
	}

	return target;
}

// ---- JSON Patch (RFC 6902) ----

namespace
{
	using sp::json::Value;

	Value& descend(Value& node, const String& token)
	{
		if (node.is_array())
		{
			std::size_t index = 0;

			if (!parse_index(token, index) || index >= node.as_array().size())
				throw sp::Invalid_Operation("JSON Patch: array token not found: " + token);

			return node.array_items()[index];
		}

		if (node.is_object())
		{
			for (auto& entry : node.object_items())
			{
				if (entry.first == sp::cppString{ token })
					return entry.second;
			}

			throw sp::Invalid_Operation("JSON Patch: object key not found: " + token);
		}

		throw sp::Invalid_Operation("JSON Patch: cannot traverse a scalar value.");
	}

	struct Location
	{
		Value* parent = nullptr;   // nullptr denotes the whole document
		String token;
	};

	Location locate(Value& root, const String& path)
	{
		const auto tokens = pointer_tokens(path);

		if (tokens.empty())
			return {};

		Value* current = &root;

		for (std::size_t i = 0; i + 1 < tokens.size(); ++i)
			current = &descend(*current, tokens[i]);

		return { current, tokens.back() };
	}

	void add_at(Value& root, const String& path, Value value)
	{
		if (path.empty())
		{
			root = std::move(value);
			return;
		}

		const Location loc = locate(root, path);
		Value& parent = *loc.parent;

		if (parent.is_array())
		{
			auto& array = parent.array_items();

			if (loc.token == "-")
			{
				array.push_back(std::move(value));
				return;
			}

			std::size_t index = 0;

			if (!parse_index(loc.token, index) || index > array.size())
				throw sp::Invalid_Operation("JSON Patch add: invalid array index.");

			array.insert(array.begin() + static_cast<std::ptrdiff_t>(index), std::move(value));
			return;
		}

		if (!parent.is_object() && !parent.is_null())
			throw sp::Invalid_Operation("JSON Patch add: parent is not an object.");

		auto& object = parent.object_items();

		for (auto& entry : object)
		{
			if (entry.first == sp::cppString{ loc.token })
			{
				entry.second = std::move(value);
				return;
			}
		}

		object.emplace_back(sp::cppString{ loc.token }, std::move(value));
	}

	void remove_at(Value& root, const String& path)
	{
		if (path.empty())
		{
			root = Value{};
			return;
		}

		const Location loc = locate(root, path);
		Value& parent = *loc.parent;

		if (parent.is_array())
		{
			std::size_t index = 0;

			if (!parse_index(loc.token, index))
				throw sp::Invalid_Operation("JSON Patch remove: non-numeric array token.");

			auto& array = parent.array_items();

			if (index >= array.size())
				throw sp::Index_Out_Of_Range("JSON Patch remove: array index out of range.");

			array.erase(array.begin() + static_cast<std::ptrdiff_t>(index));
			return;
		}

		if (!parent.is_object())
			throw sp::Invalid_Operation("JSON Patch remove: parent is not an object.");

		auto& object = parent.object_items();

		for (std::size_t i = 0; i < object.size(); ++i)
		{
			if (object[i].first == sp::cppString{ loc.token })
			{
				object.erase(object.begin() + static_cast<std::ptrdiff_t>(i));
				return;
			}
		}

		throw sp::Invalid_Operation("JSON Patch remove: path not found.");
	}

	bool is_descendant_path(const String& path, const String& from) noexcept
	{
		return path.size() > from.size()
			&& path.compare(0, from.size(), from) == 0
			&& path[from.size()] == '/';
	}
}

sp::json::Value sp::json::patch(Value document, const Value& operations)
{
	if (!operations.is_array())
		throw Invalid_Conversion("JSON Patch must be an array of operation objects.");

	for (const Value& op : operations.as_array())
	{
		if (!op.is_object())
			throw Invalid_Conversion("Every JSON Patch operation must be an object.");

		if (!op.contains("op") || !op["op"].is_string()
			|| !op.contains("path") || !op["path"].is_string())
		{
			throw Invalid_Conversion("JSON Patch operation needs string 'op' and 'path'.");
		}

		const String name = op["op"].as_string().s_str();
		const String path = op["path"].as_string().s_str();

		if (name == "add" || name == "replace" || name == "test")
		{
			if (!op.contains("value"))
				throw Invalid_Conversion("JSON Patch '" + name + "' needs a 'value'.");

			if (name == "test")
			{
				const Value& actual = pointer(document, sp::cppString{ path });

				if (!(actual == op["value"]))
					throw Invalid_Operation("JSON Patch test failed at " + path);
			}
			else if (name == "replace")
			{
				(void)pointer(document, sp::cppString{ path }); // must already exist
				set_pointer(document, sp::cppString{ path }, op["value"]);
			}
			else
			{
				add_at(document, path, op["value"]);
			}
		}
		else if (name == "remove")
		{
			(void)pointer(document, sp::cppString{ path }); // must already exist
			remove_at(document, path);
		}
		else if (name == "move" || name == "copy")
		{
			if (!op.contains("from") || !op["from"].is_string())
				throw Invalid_Conversion("JSON Patch '" + name + "' needs a 'from' path.");

			const String from = op["from"].as_string().s_str();
			Value taken = pointer(document, sp::cppString{ from });

			if (name == "move")
			{
				if (is_descendant_path(path, from))
					throw Invalid_Operation("JSON Patch move: cannot move a value into its own child.");

				remove_at(document, from);
			}

			add_at(document, path, std::move(taken));
		}
		else
		{
			throw Invalid_Conversion("Unknown JSON Patch operation: " + name);
		}
	}

	return document;
}
