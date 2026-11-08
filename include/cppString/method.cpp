// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "method.h"
#include "utf8.h"
#include <fstream>
#include <iterator>
#include <random>

void sp::reverse_string(cppString& str)
{
	str.reverse();
}

void sp::reverse_string_to(cppString& str, cppString& result)
{
	result = str;
	result.reverse();
}

void sp::reverse_string_to(cppString&& str, cppString& result) noexcept
{
	result = std::move(str);
	result.reverse();
}

void sp::copy_to(cppString& str, cppString& result)
{
	result = str;
}

void sp::copy_to(cppString&& str, cppString& result) noexcept
{
	result = std::move(str);
}

void sp::move_to(cppString& str, cppString& result)
{
	result = std::move(str);
	str.reset();
}

void sp::move_to(cppString&& str, cppString& result) noexcept
{
	result = std::exchange(str, cppString{});
}

void sp::fill_with(cppString& str, char ch)
{
	str.reset(std::string(str.size(), ch));
}

void sp::fill(cppString& str, char ch, std::size_t count)
{
	str.reset(std::string(count, ch));
}

void sp::remove_prefix(cppString& str, const cppString& prefix)
{
	str.remove_prefix(prefix);
}

void sp::remove_suffix(cppString& str, const cppString& suffix)
{
	str.remove_suffix(suffix);
}

sp::cppString sp::join(const std::vector<cppString>& parts, const cppString& separator)
{
	cppString result;

	for (std::size_t i = 0; i < parts.size(); ++i)
	{
		if (i != 0)
			result += separator;

		result += parts[i];
	}

	return result;
}

sp::cppString sp::join(const std::vector<cppString>& parts, char separator)
{
	return sp::join(parts, cppString(String(1, separator)));
}

std::vector<sp::cppString> sp::split_lines(const cppString& str)
{
	std::vector<cppString> lines;

	const String& src = str.s_str();
	std::size_t start = 0;
	std::size_t i = 0;

	while (i < src.size())
	{
		const char c = src[i];

		if (c == '\n' || c == '\r')
		{
			lines.emplace_back(src.substr(start, i - start));

			if (c == '\r' && i + 1 < src.size() && src[i + 1] == '\n')
				++i;

			start = i + 1;
		}

		++i;
	}

	if (start < src.size())
		lines.emplace_back(src.substr(start));

	return lines;
}

void sp::pad_left(cppString& str, std::size_t width, char fill)
{
	const std::size_t count = utf8::length(str);
	if (count >= width)
		return;

	str.insert(0, String(count < width ? width - count : 0, fill));
}

void sp::pad_right(cppString& str, std::size_t width, char fill)
{
	const std::size_t count = utf8::length(str);
	if (count >= width)
		return;

	str.append(width - count, fill);
}

void sp::capitalize(cppString& str)
{
	if (str.empty())
		return;

	String& data = str.get_changable_string();

	data[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(data[0])));

	for (std::size_t i = 1; i < data.size(); ++i)
		data[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(data[i])));
}

void sp::swap_case(cppString& str)
{
	String& data = str.get_changable_string();

	for (auto& ch : data)
	{
		const unsigned char c = static_cast<unsigned char>(ch);

		if (std::islower(c))
			ch = static_cast<char>(std::toupper(c));
		else if (std::isupper(c))
			ch = static_cast<char>(std::tolower(c));
	}
}

void sp::replace_first(cppString& str, const cppString& from, const cppString& to)
{
	if (from.empty())
		throw sp::Invalid_Argument("The substring to replace cannot be empty.");

	const auto pos = str.find(from);

	if (pos != cppString::npos)
		str.replace(pos, from.size(), to);
}

sp::cppString sp::read_file(const String& path)
{
	std::ifstream stream(path, std::ios::binary);

	if (!stream.is_open())
		throw sp::Invalid_Operation("Cannot open file for reading: " + path);

	String data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

	return cppString(std::move(data));
}

std::vector<sp::cppString> sp::read_lines(const String& path)
{
	return sp::split_lines(sp::read_file(path));
}

void sp::write_file(const String& path, const cppString& data, bool append)
{
	std::ofstream stream(
		path,
		std::ios::binary | (append ? std::ios::app : std::ios::trunc)
	);

	if (!stream.is_open())
		throw sp::Invalid_Operation("Cannot open file for writing: " + path);

	stream.write(data.data(), static_cast<std::streamsize>(data.size()));

	if (!stream)
		throw sp::Invalid_Operation("Failed while writing file: " + path);
}

// ---- random ----

sp::cppString sp::generate_random(std::size_t length, const cppString& charset)
{
	const String& chars = charset.s_str();

	if (chars.empty())
		throw sp::Invalid_Argument("Charset cannot be empty in generate_random.");

	static thread_local std::mt19937 generator{ std::random_device{}() };

	std::uniform_int_distribution<std::size_t> pick(0, chars.size() - 1);

	String result;
	result.reserve(length);

	for (std::size_t i = 0; i < length; ++i)
		result += chars[pick(generator)];

	return cppString(std::move(result));
}

// ---- case-insensitive conveniences (ASCII only) ----

namespace
{
	String to_lower_ascii(String s)
	{
		for (auto& c : s)
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

		return s;
	}
}

bool sp::icontains(const cppString& str, const cppString& sub)
{
	const String haystack = to_lower_ascii(str.s_str());
	const String needle = to_lower_ascii(sub.s_str());

	return haystack.find(needle) != String::npos;
}

bool sp::istarts_with(const cppString& str, const cppString& prefix)
{
	const String haystack = to_lower_ascii(str.s_str());
	const String needle = to_lower_ascii(prefix.s_str());

	return haystack.starts_with(needle);
}

bool sp::iends_with(const cppString& str, const cppString& suffix)
{
	const String haystack = to_lower_ascii(str.s_str());
	const String needle = to_lower_ascii(suffix.s_str());

	return haystack.ends_with(needle);
}

// ---- splitting ----

std::vector<sp::cppString> sp::split_by_any(const cppString& str, const cppString& delimiters)
{
	const String& src = str.s_str();
	const String& delims = delimiters.s_str();

	if (delims.empty())
		throw sp::Invalid_Argument("Delimiters cannot be empty in split_by_any.");

	std::vector<cppString> fields;

	std::size_t start = 0;

	while (true)
	{
		const std::size_t end = src.find_first_of(delims, start);

		if (end == String::npos)
		{
			fields.emplace_back(src.substr(start));
			break;
		}

		fields.emplace_back(src.substr(start, end - start));
		start = end + 1;
	}

	// A trailing delimiter produces no trailing empty field.
	if (!fields.empty() && fields.back().empty() && !src.empty()
		&& delims.find(src.back()) != String::npos)
	{
		fields.pop_back();
	}

	return fields;
}

// ---- pattern matching ----

bool sp::wildcard_match(const cppString& str, const cppString& pattern)
{
	const String& s = str.s_str();
	const String& p = pattern.s_str();

	std::size_t si = 0, pi = 0;
	std::size_t star = String::npos, mark = 0;

	while (si < s.size())
	{
		if (pi < p.size() && (p[pi] == '?' || p[pi] == s[si]))
		{
			++si;
			++pi;
		}
		else if (pi < p.size() && p[pi] == '*')
		{
			star = pi++;
			mark = si;
		}
		else if (star != String::npos)
		{
			pi = star + 1;
			si = ++mark;
		}
		else
		{
			return false;
		}
	}

	while (pi < p.size() && p[pi] == '*')
		++pi;

	return pi == p.size();
}

// ---- case style conversions ----

namespace
{
	bool is_separator(char c)
	{
		return c == ' ' || c == '-' || c == '_';
	}

	// Word boundaries: separators, or an uppercase letter that follows a lowercase letter or digit.
	std::vector<String> split_words(const String& src)
	{
		std::vector<String> words;
		String current;

		for (std::size_t i = 0; i < src.size(); ++i)
		{
			const char c = src[i];

			if (is_separator(c))
			{
				if (!current.empty())
				{
					words.push_back(std::move(current));
					current.clear();
				}
				continue;
			}

			const bool boundary =
				std::isupper(static_cast<unsigned char>(c))
				&& !current.empty()
				&& (std::islower(static_cast<unsigned char>(current.back()))
					|| std::isdigit(static_cast<unsigned char>(current.back())));

			if (boundary)
			{
				words.push_back(std::move(current));
				current.clear();
			}

			current += c;
		}

		if (!current.empty())
			words.push_back(std::move(current));

		return words;
	}
}

void sp::to_snake_case(cppString& str)
{
	const auto words = split_words(str.s_str());

	String result;

	for (std::size_t i = 0; i < words.size(); ++i)
	{
		if (i > 0)
			result += '_';

		result += to_lower_ascii(words[i]);
	}

	str = cppString(std::move(result));
}

void sp::to_kebab_case(cppString& str)
{
	const auto words = split_words(str.s_str());

	String result;

	for (std::size_t i = 0; i < words.size(); ++i)
	{
		if (i > 0)
			result += '-';

		result += to_lower_ascii(words[i]);
	}

	str = cppString(std::move(result));
}

void sp::to_camel_case(cppString& str)
{
	const auto words = split_words(str.s_str());

	String result;

	for (std::size_t i = 0; i < words.size(); ++i)
	{
		String word = to_lower_ascii(words[i]);

		if (i > 0 && !word.empty())
			word[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));

		result += word;
	}

	str = cppString(std::move(result));
}

void sp::to_pascal_case(cppString& str)
{
	const auto words = split_words(str.s_str());

	String result;

	for (const auto& raw : words)
	{
		String word = to_lower_ascii(raw);

		if (!word.empty())
			word[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));

		result += word;
	}

	str = cppString(std::move(result));
}

// ---- JSON ----

sp::cppString sp::json::escape(const cppString& text)
{
	static const char* const HEX = "0123456789abcdef";

	String result;
	result.reserve(text.size() + 2);

	for (const char c : text.s_str())
	{
		const unsigned char uc = static_cast<unsigned char>(c);

		switch (c)
		{
		case '"':  result += "\\\""; continue;
		case '\\': result += "\\\\"; continue;
		case '\b': result += "\\b";  continue;
		case '\f': result += "\\f";  continue;
		case '\n': result += "\\n";  continue;
		case '\r': result += "\\r";  continue;
		case '\t': result += "\\t";  continue;
		default: break;
		}

		if (uc < 0x20)
		{
			result += "\\u00";
			result += HEX[(uc >> 4) & 0xF];
			result += HEX[uc & 0xF];
		}
		else
		{
			result += c;
		}
	}

	return cppString(std::move(result));
}

namespace
{
	// Decode one \uXXXX sequence starting at src[i] (i points at 'u').
	// Returns the code unit and advances i past the 4 hex digits.
	unsigned json_hex4(const String& src, std::size_t& i)
	{
		if (i + 4 >= src.size())
			throw sp::Invalid_Conversion("Truncated \\u escape in json::unescape.");

		unsigned value = 0;

		for (int k = 0; k < 4; ++k)
		{
			const char c = src[++i];
			unsigned digit;

			if (c >= '0' && c <= '9')      digit = static_cast<unsigned>(c - '0');
			else if (c >= 'a' && c <= 'f') digit = static_cast<unsigned>(c - 'a' + 10);
			else if (c >= 'A' && c <= 'F') digit = static_cast<unsigned>(c - 'A' + 10);
			else
				throw sp::Invalid_Conversion("Invalid hex digit in \\u escape in json::unescape.");

			value = (value << 4) | digit;
		}

		return value;
	}

	void append_utf8(String& out, unsigned codepoint)
	{
		if (codepoint <= 0x7F)
		{
			out += static_cast<char>(codepoint);
		}
		else if (codepoint <= 0x7FF)
		{
			out += static_cast<char>(0xC0 | (codepoint >> 6));
			out += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else if (codepoint <= 0xFFFF)
		{
			out += static_cast<char>(0xE0 | (codepoint >> 12));
			out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else
		{
			out += static_cast<char>(0xF0 | (codepoint >> 18));
			out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
			out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
	}
}

sp::cppString sp::json::unescape(const cppString& text)
{
	const String& src = text.s_str();

	String result;
	result.reserve(src.size());

	for (std::size_t i = 0; i < src.size(); ++i)
	{
		if (src[i] != '\\')
		{
			result += src[i];
			continue;
		}

		if (++i >= src.size())
			throw sp::Invalid_Conversion("Trailing backslash in json::unescape.");

		switch (src[i])
		{
		case '"':  result += '"';  break;
		case '\\': result += '\\'; break;
		case '/':  result += '/';  break;
		case 'b':  result += '\b'; break;
		case 'f':  result += '\f'; break;
		case 'n':  result += '\n'; break;
		case 'r':  result += '\r'; break;
		case 't':  result += '\t'; break;
		case 'u':
		{
			unsigned cp = json_hex4(src, i);

			if (cp >= 0xD800 && cp <= 0xDBFF) // high surrogate
			{
				if (i + 6 >= src.size() || src[i + 1] != '\\' || src[i + 2] != 'u')
					throw sp::Invalid_Conversion("Unpaired high surrogate in json::unescape.");

				i += 2;

				const unsigned low = json_hex4(src, i);

				if (low < 0xDC00 || low > 0xDFFF)
					throw sp::Invalid_Conversion("Invalid low surrogate in json::unescape.");

				cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
			}
			else if (cp >= 0xDC00 && cp <= 0xDFFF)
			{
				throw sp::Invalid_Conversion("Unpaired low surrogate in json::unescape.");
			}

			append_utf8(result, cp);
			break;
		}
		default:
			throw sp::Invalid_Conversion("Unknown escape sequence in json::unescape.");
		}
	}

	return cppString(std::move(result));
}

// ---- CSV ----

sp::cppString sp::csv::escape(const cppString& field)
{
	const String& src = field.s_str();

	const bool needs_quotes =
		src.find_first_of(",\"\r\n") != String::npos;

	if (!needs_quotes)
		return field;

	String result;
	result.reserve(src.size() + 2);

	result += '"';

	for (const char c : src)
	{
		if (c == '"')
			result += '"';

		result += c;
	}

	result += '"';

	return cppString(std::move(result));
}

std::vector<sp::cppString> sp::csv::split(const cppString& line)
{
	const String& src = line.s_str();

	std::vector<cppString> fields;

	std::size_t i = 0;

	while (true)
	{
		String field;

		if (i < src.size() && src[i] == '"') // quoted field
		{
			++i;

			while (true)
			{
				if (i >= src.size())
					throw sp::Invalid_Conversion("Unterminated quoted field in csv::split.");

				if (src[i] == '"')
				{
					if (i + 1 < src.size() && src[i + 1] == '"') // doubled quote
					{
						field += '"';
						i += 2;
						continue;
					}

					++i;
					break;
				}

				field += src[i++];
			}

			if (i < src.size() && src[i] != ',')
				throw sp::Invalid_Conversion("Unexpected characters after closing quote in csv::split.");
		}
		else // bare field
		{
			while (i < src.size() && src[i] != ',')
				field += src[i++];
		}

		fields.emplace_back(std::move(field));

		if (i >= src.size())
			break;

		++i; // skip comma
	}

	return fields;
}

// ---- statistics ----

std::size_t sp::word_count(const cppString& str)
{
	std::size_t words = 0;
	bool in_word = false;

	for (const char c : str.s_str())
	{
		const bool space = std::isspace(static_cast<unsigned char>(c)) != 0;

		if (space)
		{
			in_word = false;
		}
		else if (!in_word)
		{
			in_word = true;
			++words;
		}
	}

	return words;
}

std::size_t sp::levenshtein(const cppString& a, const cppString& b)
{
	const auto lhs = utf8::chars(a);
	const auto rhs = utf8::chars(b);

	const std::size_t m = lhs.size();
	const std::size_t n = rhs.size();

	if (m == 0) return n;
	if (n == 0) return m;

	std::vector<std::size_t> previous(n + 1);
	std::vector<std::size_t> current(n + 1);

	for (std::size_t j = 0; j <= n; ++j)
		previous[j] = j;

	for (std::size_t i = 1; i <= m; ++i)
	{
		current[0] = i;

		for (std::size_t j = 1; j <= n; ++j)
		{
			const std::size_t cost = (lhs[i - 1] == rhs[j - 1]) ? 0 : 1;

			current[j] = (std::min)({
				previous[j] + 1,        // deletion
				current[j - 1] + 1,     // insertion
				previous[j - 1] + cost  // substitution
			});
		}

		previous.swap(current);
	}

	return previous[n];
}

double sp::similarity(const cppString& a, const cppString& b)
{
	const std::size_t longest = (std::max)(utf8::length(a), utf8::length(b));

	if (longest == 0)
		return 1.0;

	return 1.0 - static_cast<double>(levenshtein(a, b)) / static_cast<double>(longest);
}

sp::cppString sp::longest_common_prefix(const std::vector<cppString>& parts)
{
	if (parts.empty())
		return cppString{};

	const String* first = &parts.front().s_str();

	std::size_t limit = first->size();

	for (const auto& part : parts)
		limit = (std::min)(limit, part.size());

	std::size_t length = 0;

	while (length < limit)
	{
		bool same = true;

		for (std::size_t i = 1; i < parts.size(); ++i)
		{
			if (parts[i].s_str()[length] != (*first)[length])
			{
				same = false;
				break;
			}
		}

		if (!same)
			break;

		++length;
	}

	// Never cut in the middle of a UTF-8 sequence.
	while (length > 0 && length < first->size()
		&& (static_cast<unsigned char>((*first)[length]) & 0xC0) == 0x80)
	{
		--length;
	}

	return cppString{ first->substr(0, length) };
}

// ---- layout ----

std::vector<sp::cppString> sp::wrap(const cppString& str, std::size_t width)
{
	if (width == 0)
		throw sp::Invalid_Argument("Width must be greater than zero in wrap.");

	std::vector<cppString> lines;

	for (const auto& line : split_lines(str))
	{
		if (line.empty())
		{
			lines.emplace_back();
			continue;
		}

		cppString current;
		std::size_t current_width = 0;

		for (const auto& word : line.split(' '))
		{
			const std::size_t word_width = utf8::length(word);

			if (current.empty())
			{
				current = word;
				current_width = word_width;
			}
			else if (current_width + 1 + word_width <= width)
			{
				current += ' ';
				current += word;
				current_width += 1 + word_width;
			}
			else
			{
				lines.push_back(current);
				current = word;
				current_width = word_width;
			}
		}

		if (!current.empty())
			lines.push_back(current);
	}

	return lines;
}

void sp::indent(cppString& str, const cppString& prefix)
{
	const auto lines = split_lines(str);

	String result;

	for (std::size_t i = 0; i < lines.size(); ++i)
	{
		if (i > 0)
			result += '\n';

		if (!lines[i].empty())
			result += prefix.s_str();

		result += lines[i].s_str();
	}

	str = cppString(std::move(result));
}

sp::cppString sp::thousands_sep(long long value, char sep)
{
	const bool negative = value < 0;
	const unsigned long long magnitude = negative
		? 0ULL - static_cast<unsigned long long>(value)
		: static_cast<unsigned long long>(value);

	String digits = std::to_string(magnitude);
	String grouped;

	int counter = 0;

	for (auto it = digits.rbegin(); it != digits.rend(); ++it)
	{
		if (counter > 0 && counter % 3 == 0)
			grouped += sep;

		grouped += *it;
		++counter;
	}

	std::ranges::reverse(grouped);

	return cppString{ (negative ? String("-") : String()) + grouped };
}

sp::cppString sp::human_size(unsigned long long bytes)
{
	static constexpr const char* UNITS[] = { "B", "KB", "MB", "GB", "TB", "PB" };

	if (bytes < 1024)
		return cppString{ std::to_string(bytes) + " B" };

	double value = static_cast<double>(bytes);
	std::size_t unit = 0;

	while (value >= 1024.0 && unit + 1 < std::size(UNITS))
	{
		value /= 1024.0;
		++unit;
	}

	cppString number{ std::format("{:.1f}", value) };

	if (number.endsWith(".0"))
		number.resize(number.size() - 2);

	return cppString{ number.s_str() + " " + UNITS[unit] };
}

sp::cppString sp::render(const cppString& tmpl, const std::map<cppString, cppString>& vars)
{
	const String& src = tmpl.s_str();

	String result;

	std::size_t pos = 0;

	while (pos < src.size())
	{
		const std::size_t open = src.find("{{", pos);

		if (open == String::npos)
		{
			result += src.substr(pos);
			break;
		}

		const std::size_t close = src.find("}}", open + 2);

		if (close == String::npos)
		{
			result += src.substr(pos);
			break;
		}

		result += src.substr(pos, open - pos);

		const String key = src.substr(open + 2, close - (open + 2));
		const auto it = vars.find(cppString{ key });

		if (it != vars.end())
			result += it->second.s_str();
		else
			result += src.substr(open, close + 2 - open); // leave unknown placeholders intact

		pos = close + 2;
	}

	return cppString(std::move(result));
}

std::pair<sp::cppString, sp::cppString> sp::split_pair(
	const cppString& str, const cppString& delimiter, bool first)
{
	if (delimiter.empty())
		throw sp::Invalid_Argument("Delimiter cannot be empty in split_pair.");

	const std::size_t pos = first ? str.find(delimiter) : str.rfind(delimiter);

	if (pos == cppString::npos)
		return { str, cppString{} };

	return {
		str.substr(0, pos),
		str.substr(pos + delimiter.size())
	};
}

// ---- convenience ----

void sp::truncate(cppString& str, std::size_t max_chars, const cppString& ellipsis)
{
	const std::size_t length = utf8::length(str);

	if (length <= max_chars)
		return;

	const std::size_t ellipsis_len = utf8::length(ellipsis);

	if (max_chars <= ellipsis_len)
	{
		str = utf8::sub(str, 0, max_chars);
		return;
	}

	str = utf8::sub(str, 0, max_chars - ellipsis_len) + ellipsis;
}

void sp::slug(cppString& str)
{
	String result;
	result.reserve(str.size());

	bool last_was_dash = true; // treat the start as a dash to trim leading separators

	for (const char c : str.s_str())
	{
		const unsigned char uc = static_cast<unsigned char>(c);

		if (std::isalnum(uc))
		{
			result += static_cast<char>(std::tolower(uc));
			last_was_dash = false;
		}
		else if ((c == ' ' || c == '_' || c == '-') && !last_was_dash)
		{
			result += '-';
			last_was_dash = true;
		}
		// every other character is dropped
	}

	while (!result.empty() && result.back() == '-')
		result.pop_back();

	str = cppString(std::move(result));
}

std::vector<sp::cppString> sp::chunk(const cppString& str, std::size_t size)
{
	if (size == 0)
		throw sp::Invalid_Argument("Chunk size must be greater than zero in chunk.");

	const std::size_t length = utf8::length(str);

	std::vector<cppString> chunks;
	chunks.reserve((length + size - 1) / size);

	for (std::size_t start = 0; start < length; start += size)
		chunks.push_back(utf8::sub(str, start, (std::min)(size, length - start)));

	return chunks;
}

bool sp::to_bool(const cppString& str)
{
	String lower = str.s_str();
	std::ranges::transform(lower, lower.begin(), [](char c) {
		return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	});

	if (lower == "true" || lower == "1" || lower == "yes"
		|| lower == "on" || lower == "y" || lower == "t")
		return true;

	if (lower == "false" || lower == "0" || lower == "no"
		|| lower == "off" || lower == "n" || lower == "f")
		return false;

	throw sp::Invalid_Conversion("Cannot convert string to bool in to_bool.");
}

// ---- string distance & diff helpers ----

std::size_t sp::hamming(const cppString& a, const cppString& b)
{
	const auto lhs = utf8::chars(a);
	const auto rhs = utf8::chars(b);

	if (lhs.size() != rhs.size())
		throw sp::Invalid_Argument("Strings must have equal character counts in hamming.");

	std::size_t distance = 0;

	for (std::size_t i = 0; i < lhs.size(); ++i)
	{
		if (lhs[i] != rhs[i])
			++distance;
	}

	return distance;
}

sp::cppString sp::common_prefix(const cppString& a, const cppString& b)
{
	const String& lhs = a.s_str();
	const String& rhs = b.s_str();

	const std::size_t limit = (std::min)(lhs.size(), rhs.size());

	std::size_t length = 0;
	while (length < limit && lhs[length] == rhs[length])
		++length;

	// Never cut in the middle of a UTF-8 sequence.
	while (length > 0
		&& (static_cast<unsigned char>(lhs[length]) & 0xC0) == 0x80)
	{
		--length;
	}

	return cppString{ lhs.substr(0, length) };
}

sp::cppString sp::common_suffix(const cppString& a, const cppString& b)
{
	const String& lhs = a.s_str();
	const String& rhs = b.s_str();

	const std::size_t limit = (std::min)(lhs.size(), rhs.size());

	std::size_t length = 0;
	while (length < limit
		&& lhs[lhs.size() - 1 - length] == rhs[rhs.size() - 1 - length])
	{
		++length;
	}

	// The suffix must start on a character boundary: the byte right before
	// the suffix must not be a continuation byte of a split sequence.
	while (length > 0
		&& (static_cast<unsigned char>(lhs[lhs.size() - length]) & 0xC0) == 0x80)
	{
		--length;
	}

	return cppString{ lhs.substr(lhs.size() - length) };
}

// ---- formatting ----

sp::cppString sp::human_duration(unsigned long long seconds)
{
	if (seconds == 0)
		return cppString{ "0s" };

	struct Unit { unsigned long long size; const char* suffix; };
	static constexpr Unit UNITS[] = {
		{ 86400, "d" }, { 3600, "h" }, { 60, "m" }, { 1, "s" },
	};

	String result;

	for (const auto& unit : UNITS)
	{
		if (seconds < unit.size)
			continue;

		const unsigned long long count = seconds / unit.size;
		seconds %= unit.size;

		if (!result.empty())
			result += ' ';

		result += std::to_string(count);
		result += unit.suffix;
	}

	return cppString(std::move(result));
}

sp::cppString sp::to_roman(int value)
{
	if (value < 1 || value > 3999)
		throw sp::Invalid_Argument("Roman numerals only cover 1..3999 in to_roman.");

	static constexpr struct { int value; const char* numeral; } TABLE[] = {
		{ 1000, "M"  }, { 900, "CM" }, { 500, "D"  }, { 400, "CD" },
		{ 100,  "C"  }, { 90,  "XC" }, { 50,  "L"  }, { 40,  "XL" },
		{ 10,   "X"  }, { 9,   "IX" }, { 5,   "V"  }, { 4,   "IV" },
		{ 1,    "I"  },
	};

	String result;

	for (const auto& entry : TABLE)
	{
		while (value >= entry.value)
		{
			result += entry.numeral;
			value -= entry.value;
		}
	}

	return cppString(std::move(result));
}

int sp::from_roman(const cppString& text)
{
	const String& src = text.s_str();

	if (src.empty())
		throw sp::Invalid_Conversion("Empty string in from_roman.");

	auto digit_value = [](char c) -> int
	{
		switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c))))
		{
		case 'M': return 1000;
		case 'D': return 500;
		case 'C': return 100;
		case 'L': return 50;
		case 'X': return 10;
		case 'V': return 5;
		case 'I': return 1;
		default:  return 0;
		}
	};

	int total = 0;
	int previous = 0;

	for (auto it = src.rbegin(); it != src.rend(); ++it)
	{
		const int value = digit_value(*it);

		if (value == 0)
			throw sp::Invalid_Conversion("Invalid Roman numeral character in from_roman.");

		if (value < previous)
			total -= value;
		else
			total += value;

		previous = value;
	}

	// Reject non-canonical forms like "IIII", "VX", "IL".
	if (total < 1 || total > 3999 || to_roman(total).s_str() != [&] {
			String upper = src;
			for (auto& c : upper)
				c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
			return upper;
		}())
	{
		throw sp::Invalid_Conversion("Malformed Roman numeral in from_roman.");
	}

	return total;
}

// ---- HTML ----

sp::cppString sp::html::escape(const cppString& text)
{
	String result;
	result.reserve(text.size());

	for (const char c : text.s_str())
	{
		switch (c)
		{
		case '&':  result += "&amp;";  break;
		case '<':  result += "&lt;";   break;
		case '>':  result += "&gt;";   break;
		case '\"': result += "&quot;"; break;
		case '\'': result += "&#39;";  break;
		default:   result += c;        break;
		}
	}

	return cppString(std::move(result));
}

sp::cppString sp::html::unescape(const cppString& text)
{
	const String& src = text.s_str();

	String result;
	result.reserve(src.size());

	auto named = [](const String& name) -> const char*
	{
		if (name == "amp")   return "&";
		if (name == "lt")    return "<";
		if (name == "gt")    return ">";
		if (name == "quot")  return "\"";
		if (name == "apos")  return "'";
		if (name == "nbsp")  return "\xC2\xA0"; // U+00A0 in UTF-8
		return nullptr;
	};

	for (std::size_t i = 0; i < src.size(); ++i)
	{
		if (src[i] != '&')
		{
			result += src[i];
			continue;
		}

		const std::size_t semi = src.find(';', i + 1);

		if (semi == String::npos || semi - i > 10)
			throw sp::Invalid_Conversion("Unterminated entity in html::unescape.");

		const String body = src.substr(i + 1, semi - i - 1);

		if (body.empty())
			throw sp::Invalid_Conversion("Empty entity in html::unescape.");

		if (body[0] == '#') // numeric entity
		{
			unsigned codepoint = 0;
			std::size_t k = 1;

			const bool hex = k < body.size() && (body[k] == 'x' || body[k] == 'X');

			if (hex)
				++k;

			if (k == body.size())
				throw sp::Invalid_Conversion("Empty numeric entity in html::unescape.");

			for (; k < body.size(); ++k)
			{
				const unsigned char c = static_cast<unsigned char>(body[k]);
				unsigned digit;

				if (c >= '0' && c <= '9')
					digit = static_cast<unsigned>(c - '0');
				else if (hex && c >= 'a' && c <= 'f')
					digit = static_cast<unsigned>(c - 'a' + 10);
				else if (hex && c >= 'A' && c <= 'F')
					digit = static_cast<unsigned>(c - 'A' + 10);
				else
					throw sp::Invalid_Conversion("Bad digit in numeric entity in html::unescape.");

				codepoint = hex ? codepoint * 16 + digit : codepoint * 10 + digit;

				if (codepoint > 0x10FFFF)
					throw sp::Invalid_Conversion("Entity out of range in html::unescape.");
			}

			if (codepoint == 0 || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
				throw sp::Invalid_Conversion("Invalid codepoint in html::unescape.");

			// Encode the codepoint as UTF-8.
			if (codepoint <= 0x7F)
				result += static_cast<char>(codepoint);
			else if (codepoint <= 0x7FF)
			{
				result += static_cast<char>(0xC0 | (codepoint >> 6));
				result += static_cast<char>(0x80 | (codepoint & 0x3F));
			}
			else if (codepoint <= 0xFFFF)
			{
				result += static_cast<char>(0xE0 | (codepoint >> 12));
				result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
				result += static_cast<char>(0x80 | (codepoint & 0x3F));
			}
			else
			{
				result += static_cast<char>(0xF0 | (codepoint >> 18));
				result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
				result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
				result += static_cast<char>(0x80 | (codepoint & 0x3F));
			}
		}
		else // named entity
		{
			const char* replacement = named(body);

			if (replacement == nullptr)
				throw sp::Invalid_Conversion("Unknown named entity in html::unescape.");

			result += replacement;
		}

		i = semi;
	}

	return cppString(std::move(result));
}

// ---- more text tools ----

void sp::title_case(cppString& str)
{
	String result = str.s_str();

	bool at_word_start = true;

	for (auto& c : result)
	{
		const unsigned char uc = static_cast<unsigned char>(c);

		if (std::isalpha(uc))
		{
			c = static_cast<char>(at_word_start ? std::toupper(uc) : std::tolower(uc));
			at_word_start = false;
		}
		else
		{
			at_word_start = true;
		}
	}

	str = cppString(std::move(result));
}

sp::cppString sp::center(const cppString& str, std::size_t width, char fill)
{
	const std::size_t length = utf8::length(str);

	if (length >= width)
		return str;

	const std::size_t total = width - length;
	const std::size_t left = total / 2;
	const std::size_t right = total - left; // the odd one goes right

	String result;
	result.reserve(str.size() + total);
	result.append(left, fill);
	result += str.s_str();
	result.append(right, fill);

	return cppString(std::move(result));
}

void sp::expand_tabs(cppString& str, std::size_t tab_size)
{
	if (tab_size == 0)
		throw sp::Invalid_Argument("Tab size must be greater than zero in expand_tabs.");

	String result;
	result.reserve(str.size());

	std::size_t column = 0; // in UTF-8 characters

	for (const auto& ch : utf8::chars(str))
	{
		if (ch == "\t")
		{
			const std::size_t spaces = tab_size - (column % tab_size);

			result.append(spaces, ' ');
			column += spaces;
		}
		else
		{
			result += ch.s_str();
			column = (ch == "\n" || ch == "\r") ? 0 : column + 1;
		}
	}

	str = cppString(std::move(result));
}

void sp::reverse_words(cppString& str)
{
	std::vector<String> words;
	String current;

	for (const char c : str.s_str())
	{
		if (std::isspace(static_cast<unsigned char>(c)))
		{
			if (!current.empty())
			{
				words.push_back(std::move(current));
				current.clear();
			}
		}
		else
		{
			current += c;
		}
	}

	if (!current.empty())
		words.push_back(std::move(current));

	String result;

	for (auto it = words.rbegin(); it != words.rend(); ++it)
	{
		if (!result.empty())
			result += ' ';

		result += *it;
	}

	str = cppString(std::move(result));
}

std::vector<sp::cppString> sp::ngrams(const cppString& str, std::size_t n)
{
	if (n == 0)
		throw sp::Invalid_Argument("n must be greater than zero in ngrams.");

	const auto chars = utf8::chars(str);

	std::vector<cppString> grams;

	if (chars.size() < n)
		return grams;

	grams.reserve(chars.size() - n + 1);

	for (std::size_t i = 0; i + n <= chars.size(); ++i)
	{
		String gram;

		for (std::size_t j = 0; j < n; ++j)
			gram += chars[i + j].s_str();

		grams.emplace_back(std::move(gram));
	}

	return grams;
}

bool sp::is_palindrome(const cppString& str) noexcept
{
	const auto chars = utf8::chars(str);

	for (std::size_t i = 0, j = chars.size(); i < j / 2; ++i)
	{
		if (chars[i] != chars[j - 1 - i])
			return false;
	}

	return true;
}

sp::cppString sp::soundex(const cppString& str)
{
	auto code_of = [](char c) -> char
	{
		switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c))))
		{
		case 'B': case 'F': case 'P': case 'V':              return '1';
		case 'C': case 'G': case 'J': case 'K':
		case 'Q': case 'S': case 'X': case 'Z':              return '2';
		case 'D': case 'T':                                  return '3';
		case 'L':                                            return '4';
		case 'M': case 'N':                                  return '5';
		case 'R':                                            return '6';
		default:                                             return '0';
		}
	};

	const String& src = str.s_str();

	char first = '\0';
	std::size_t start = 0;

	while (start < src.size()
		&& !std::isalpha(static_cast<unsigned char>(src[start])))
	{
		++start;
	}

	if (start == src.size())
		throw sp::Invalid_Argument("No letters in soundex.");

	first = static_cast<char>(std::toupper(static_cast<unsigned char>(src[start])));

	String digits;
	char previous_code = code_of(src[start]);

	for (std::size_t i = start + 1; i < src.size() && digits.size() < 3; ++i)
	{
		const char c = src[i];

		if (!std::isalpha(static_cast<unsigned char>(c)))
		{
			previous_code = '0'; // any non-letter separates adjacent codes
			continue;
		}

		const char up = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
		const char code = code_of(c);

		if (code != '0' && code != previous_code)
			digits += code;

		// H and W do not separate adjacent codes; vowels do.
		if (code != '0' || (up != 'H' && up != 'W'))
			previous_code = code;
	}

	digits.append(3 - digits.size(), '0');

	return cppString{ String(1, first) + digits };
}

sp::cppString sp::uuid_v4()
{
	static thread_local std::mt19937_64 generator{ std::random_device{}() };

	unsigned char bytes[16];

	for (auto& byte : bytes)
		byte = static_cast<unsigned char>(generator() & 0xFF);

	bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0F) | 0x40); // version 4
	bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3F) | 0x80); // variant 1

	static constexpr char HEX[] = "0123456789abcdef";

	String result;
	result.reserve(36);

	for (std::size_t i = 0; i < 16; ++i)
	{
		if (i == 4 || i == 6 || i == 8 || i == 10)
			result += '-';

		result += HEX[(bytes[i] >> 4) & 0xF];
		result += HEX[bytes[i] & 0xF];
	}

	return cppString(std::move(result));
}

// ---- more convenience (3.2) ----

void sp::dedent(cppString& str)
{
	const auto lines = split_lines(str);

	// Longest shared run of leading spaces/tabs across non-empty lines.
	std::size_t margin = String::npos;

	for (const auto& line : lines)
	{
		const String& s = line.s_str();

		if (s.empty())
			continue;

		std::size_t lead = 0;

		while (lead < s.size() && (s[lead] == ' ' || s[lead] == '\t'))
			++lead;

		if (lead == s.size())
			continue; // whitespace-only lines do not shrink the margin

		margin = (margin == String::npos) ? lead : (std::min)(margin, lead);
	}

	if (margin == String::npos || margin == 0)
		return; // nothing to strip (or nothing but whitespace)

	String result;

	for (std::size_t i = 0; i < lines.size(); ++i)
	{
		String s = lines[i].s_str();
		std::size_t stripped = 0;

		while (stripped < margin && stripped < s.size()
			&& (s[stripped] == ' ' || s[stripped] == '\t'))
		{
			++stripped;
		}

		result += s.substr(stripped);

		if (i + 1 < lines.size())
			result += '\n';
	}

	str = cppString(std::move(result));
}

void sp::squeeze(cppString& str, const cppString& chars)
{
	const String& src = str.s_str();

	if (src.empty())
		return;

	String result;
	result.reserve(src.size());

	char prev = src[0];
	result += prev;

	for (std::size_t i = 1; i < src.size(); ++i)
	{
		const char c = src[i];

		if (c == prev && (chars.empty() || chars.contains(c)))
			continue;

		result += c;
		prev = c;
	}

	str = cppString(std::move(result));
}

std::size_t sp::count(const cppString& str, const cppString& needle)
{
	const String& src = str.s_str();
	const String& sub = needle.s_str();

	if (sub.empty())
		throw sp::Invalid_Argument("Cannot count an empty needle in count.");

	std::size_t total = 0;
	std::size_t pos = 0;

	while ((pos = src.find(sub, pos)) != String::npos)
	{
		++total;
		pos += sub.size();
	}

	return total;
}

bool sp::is_anagram(const cppString& a, const cppString& b, bool ignore_case)
{
	String sa = a.s_str();
	String sb = b.s_str();

	if (ignore_case)
	{
		sa = a.copyLower().s_str();
		sb = b.copyLower().s_str();
	}

	if (sa.size() != sb.size())
		return false;

	std::sort(sa.begin(), sa.end());
	std::sort(sb.begin(), sb.end());

	return sa == sb;
}

double sp::dice_coefficient(const cppString& a, const cppString& b)
{
	const auto ca = utf8::chars(a);
	const auto cb = utf8::chars(b);

	if (ca.size() < 2 || cb.size() < 2)
		return a == b ? 1.0 : 0.0;

	// Multiset intersection of character bigrams.
	std::map<String, std::size_t> bigrams;

	for (std::size_t i = 0; i + 1 < ca.size(); ++i)
		++bigrams[ca[i].s_str() + ca[i + 1].s_str()];

	std::size_t overlap = 0;

	for (std::size_t i = 0; i + 1 < cb.size(); ++i)
	{
		auto it = bigrams.find(cb[i].s_str() + cb[i + 1].s_str());

		if (it != bigrams.end() && it->second > 0)
		{
			--it->second;
			++overlap;
		}
	}

	return 2.0 * static_cast<double>(overlap)
		/ static_cast<double>(ca.size() - 1 + (cb.size() - 1));
}

// ---- diff ----

std::vector<sp::DiffLine> sp::diff_lines(const cppString& old_text, const cppString& new_text)
{
	const auto old_lines = split_lines(old_text);
	const auto new_lines = split_lines(new_text);

	const std::size_t n = old_lines.size();
	const std::size_t m = new_lines.size();

	// LCS length table, (n+1) x (m+1).
	std::vector<std::size_t> lcs((n + 1) * (m + 1), 0);

	auto at = [&](std::size_t i, std::size_t j) -> std::size_t&
	{
		return lcs[i * (m + 1) + j];
	};

	for (std::size_t i = n; i-- > 0;)
	{
		for (std::size_t j = m; j-- > 0;)
		{
			if (old_lines[i] == new_lines[j])
				at(i, j) = at(i + 1, j + 1) + 1;
			else
				at(i, j) = (std::max)(at(i + 1, j), at(i, j + 1));
		}
	}

	// Walk the table: deletions first, then insertions, matches as context.
	std::vector<DiffLine> result;
	std::size_t i = 0, j = 0;

	while (i < n && j < m)
	{
		if (old_lines[i] == new_lines[j])
		{
			result.push_back({ ' ', old_lines[i] });
			++i;
			++j;
		}
		else if (at(i + 1, j) >= at(i, j + 1))
		{
			result.push_back({ '-', old_lines[i] });
			++i;
		}
		else
		{
			result.push_back({ '+', new_lines[j] });
			++j;
		}
	}

	while (i < n)
		result.push_back({ '-', old_lines[i++] });

	while (j < m)
		result.push_back({ '+', new_lines[j++] });

	return result;
}

// ---- unified diff (5.0) ----

sp::cppString sp::unified_diff(const cppString& old_text, const cppString& new_text,
	std::size_t context, const cppString& old_name, const cppString& new_name)
{
	const auto diff = diff_lines(old_text, new_text);
	const std::size_t total = diff.size();

	std::vector<bool> keep(total, false);
	bool changed = false;

	for (std::size_t i = 0; i < total; ++i)
	{
		if (diff[i].tag == ' ')
			continue;

		changed = true;
		const std::size_t low = (i > context) ? i - context : 0;
		const std::size_t high = (std::min)(total - 1, i + context);

		for (std::size_t k = low; k <= high; ++k)
			keep[k] = true;
	}

	if (!changed)
		return cppString{};

	String out;
	out += "--- ";
	out += old_name.s_str();
	out += '\n';
	out += "+++ ";
	out += new_name.s_str();
	out += '\n';

	std::size_t start = 0;

	while (start < total)
	{
		while (start < total && !keep[start])
			++start;

		if (start >= total)
			break;

		std::size_t end = start;

		while (end + 1 < total && keep[end + 1])
			++end;

		std::size_t old_line = 1;
		std::size_t new_line = 1;

		for (std::size_t k = 0; k < start; ++k)
		{
			if (diff[k].tag != '+') ++old_line;
			if (diff[k].tag != '-') ++new_line;
		}

		std::size_t old_count = 0;
		std::size_t new_count = 0;

		for (std::size_t k = start; k <= end; ++k)
		{
			if (diff[k].tag != '+') ++old_count;
			if (diff[k].tag != '-') ++new_count;
		}

		out += "@@ -" + std::to_string(old_line) + "," + std::to_string(old_count)
			+ " +" + std::to_string(new_line) + "," + std::to_string(new_count) + " @@\n";

		for (std::size_t k = start; k <= end; ++k)
		{
			out += diff[k].tag;
			out += diff[k].text.s_str();
			out += '\n';
		}

		start = end + 1;
	}

	return cppString(std::move(out));
}


sp::cppString sp::ljust(const cppString& str, std::size_t width, char fill)
{
	const std::size_t n = str.size();
	if (n >= width)
		return str;
	String out = str.s_str();
	out.append(width - n, fill);
	return cppString{ std::move(out) };
}

sp::cppString sp::rjust(const cppString& str, std::size_t width, char fill)
{
	const std::size_t n = str.size();
	if (n >= width)
		return str;
	String out(width - n, fill);
	out += str.s_str();
	return cppString{ std::move(out) };
}

sp::cppString sp::repeat(const cppString& str, std::size_t count)
{
	String out{};
	if (count != 0 && !str.empty())
		out.reserve(str.size() * count);
	for (std::size_t i = 0; i < count; ++i)
		out += str.s_str();
	return cppString{ std::move(out) };
}
