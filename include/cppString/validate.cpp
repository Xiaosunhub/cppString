// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "validate.h"
#include "json.h"

namespace
{
	bool is_hex_digit(char c) noexcept
	{
		return std::isxdigit(static_cast<unsigned char>(c)) != 0;
	}

	// Parses "a.b.c.d" strictly. Returns false on any deviation.
	bool parse_ipv4(const String& s) noexcept
	{
		if (s.empty())
			return false;

		std::size_t start = 0;
		int groups = 0;

		while (true)
		{
			const std::size_t dot = s.find('.', start);
			const String group = s.substr(start,
				dot == String::npos ? String::npos : dot - start);

			if (group.empty() || group.size() > 3)
				return false;

			if (group.size() > 1 && group[0] == '0') // no leading zeros
				return false;

			for (const char c : group)
			{
				if (!std::isdigit(static_cast<unsigned char>(c)))
					return false;
			}

			const int value = std::stoi(group);

			if (value > 255)
				return false;

			++groups;

			if (dot == String::npos)
				break;

			start = dot + 1;
		}

		return groups == 4;
	}
}

bool sp::validate::is_ipv4(const cppString& s) noexcept
{
	return parse_ipv4(s.s_str());
}

bool sp::validate::is_ipv6(const cppString& s) noexcept
{
	const String& src = s.s_str();

	if (src.empty() || src.size() > 45)
		return false;

	std::vector<String> groups;
	std::size_t pos = 0;
	bool compressed = false;

	// A leading "::" starts compressed mode.
	if (src.starts_with("::"))
	{
		compressed = true;
		pos = 2;

		if (pos == src.size()) // just "::"
			return true;
	}

	while (pos < src.size())
	{
		// Read one group: hex digits, optionally ending in an IPv4 tail.
		std::size_t end = pos;

		while (end < src.size() && src[end] != ':')
			++end;

		const String group = src.substr(pos, end - pos);

		if (group.find('.') != String::npos) // embedded IPv4 tail, must be last
		{
			if (end != src.size() || !parse_ipv4(group))
				return false;

			groups.push_back("x");
			groups.push_back("x"); // an IPv4 tail counts as two 16-bit groups
		}
		else
		{
			if (group.empty() || group.size() > 4)
				return false;

			for (const char c : group)
			{
				if (!is_hex_digit(c))
					return false;
			}

			groups.push_back(group);
		}

		if (end == src.size())
			break;

		// src[end] is the ':' that terminated this group.
		if (end + 1 >= src.size())
			return false; // trailing ':'

		if (src[end + 1] == ':') // "::" compression, legal at most once
		{
			if (compressed)
				return false;

			compressed = true;
			pos = end + 2;
		}
		else
		{
			pos = end + 1;
		}
	}

	// With "::" compression, fewer than 8 groups is fine; without, exactly 8.
	return compressed ? groups.size() < 8 : groups.size() == 8;
}

bool sp::validate::is_email(const cppString& s) noexcept
{
	const String& src = s.s_str();
	const std::size_t at = src.find('@');

	if (at == String::npos || at == 0 || at + 1 >= src.size())
		return false;

	if (src.find('@', at + 1) != String::npos)
		return false; // exactly one '@'

	const String local = src.substr(0, at);
	const String domain = src.substr(at + 1);

	if (local.size() > 64)
		return false;

	// Local part: no spaces, no control characters, not starting/ending with '.',
	// no consecutive dots. Everything else is tolerated on purpose.
	if (local.front() == '.' || local.back() == '.' || local.find("..") != String::npos)
		return false;

	for (const char c : local)
	{
		const unsigned char uc = static_cast<unsigned char>(c);
		if (uc <= 0x20 || uc == 0x7F)
			return false;
	}

	if (domain.find("..") != String::npos || domain.front() == '.' || domain.back() == '.')
		return false;

	if (domain.find('.') == String::npos)
		return false; // require at least one dot: "user@localhost" is not an e-mail here

	for (const char c : domain)
	{
		const unsigned char uc = static_cast<unsigned char>(c);
		if (uc <= 0x20 || uc == 0x7F || c == '(' || c == ')' || c == ',' || c == ':'
			|| c == ';' || c == '<' || c == '>' || c == '[' || c == ']' || c == '\\')
			return false;
	}

	return true;
}

bool sp::validate::is_uuid(const cppString& s) noexcept
{
	const String& src = s.s_str();

	if (src.size() != 36)
		return false;

	for (std::size_t i = 0; i < 36; ++i)
	{
		const bool hyphen = (i == 8 || i == 13 || i == 18 || i == 23);

		if (hyphen)
		{
			if (src[i] != '-')
				return false;
		}
		else if (!is_hex_digit(src[i]))
		{
			return false;
		}
	}

	return true;
}

bool sp::validate::is_url(const cppString& s) noexcept
{
	const String& src = s.s_str();
	const std::size_t scheme_end = src.find("://");

	if (scheme_end == String::npos || scheme_end == 0)
		return false;

	if (!std::isalpha(static_cast<unsigned char>(src[0])))
		return false;

	for (std::size_t i = 1; i < scheme_end; ++i)
	{
		const char c = src[i];

		if (!std::isalnum(static_cast<unsigned char>(c))
			&& c != '+' && c != '-' && c != '.')
		{
			return false;
		}
	}

	if (scheme_end + 3 >= src.size())
		return false; // nothing after "://"

	for (std::size_t i = scheme_end + 3; i < src.size(); ++i)
	{
		const unsigned char uc = static_cast<unsigned char>(src[i]);
		if (uc <= 0x20 || uc == 0x7F)
			return false;
	}

	return true;
}

bool sp::validate::is_mac_address(const cppString& s) noexcept
{
	const String& src = s.s_str();

	if (src.size() != 17)
		return false;

	const char sep = src[2];

	if (sep != ':' && sep != '-')
		return false;

	for (std::size_t i = 0; i < src.size(); ++i)
	{
		if (i % 3 == 2)
		{
			if (src[i] != sep)
				return false;
		}
		else if (!is_hex_digit(src[i]))
		{
			return false;
		}
	}

	return true;
}

bool sp::validate::is_hex_color(const cppString& s) noexcept
{
	const String& src = s.s_str();

	if (src.empty() || src[0] != '#')
		return false;

	const std::size_t digits = src.size() - 1;

	if (digits != 3 && digits != 4 && digits != 6 && digits != 8)
		return false;

	for (std::size_t i = 1; i < src.size(); ++i)
	{
		if (!is_hex_digit(src[i]))
			return false;
	}

	return true;
}

bool sp::validate::is_luhn(const cppString& s) noexcept
{
	const String& src = s.s_str();

	int sum = 0;
	int digits = 0;
	bool double_digit = false;

	// Walk right to left, skipping spaces and dashes.
	for (std::size_t i = src.size(); i-- > 0;)
	{
		const char c = src[i];

		if (c == ' ' || c == '-')
			continue;

		if (c < '0' || c > '9')
			return false;

		int d = c - '0';

		if (double_digit)
		{
			d *= 2;

			if (d > 9)
				d -= 9;
		}

		sum += d;
		++digits;
		double_digit = !double_digit;
	}

	return digits >= 2 && sum % 10 == 0;
}

bool sp::validate::is_ascii(const cppString& s) noexcept
{
	for (const char c : s.s_str())
	{
		if (static_cast<unsigned char>(c) >= 0x80)
			return false;
	}

	return true;
}

bool sp::validate::is_strong_password(const cppString& s) noexcept
{
	const String& src = s.s_str();

	if (src.size() < 8)
		return false;

	bool lower = false, upper = false, digit = false, symbol = false;

	for (const char c : src)
	{
		const unsigned char uc = static_cast<unsigned char>(c);

		if (uc >= 0x80)
			return false;

		if (c >= 'a' && c <= 'z')      lower = true;
		else if (c >= 'A' && c <= 'Z') upper = true;
		else if (c >= '0' && c <= '9') digit = true;
		else                           symbol = true;
	}

	return lower && upper && digit && symbol;
}

// ---- additional format validators (5.1) ----

namespace
{
	bool v_is_digit(char c) noexcept
	{
		return c >= '0' && c <= '9';
	}

	bool v_is_upper(char c) noexcept
	{
		return c >= 'A' && c <= 'Z';
	}

	bool v_is_lower(char c) noexcept
	{
		return c >= 'a' && c <= 'z';
	}

	bool v_is_alnum(char c) noexcept
	{
		return v_is_digit(c) || v_is_upper(c) || v_is_lower(c);
	}

	bool v_is_base64url(char c) noexcept
	{
		return v_is_alnum(c) || c == '-' || c == '_';
	}

	int v_base64_value(char c) noexcept
	{
		if (c >= 'A' && c <= 'Z') return c - 'A';
		if (c >= 'a' && c <= 'z') return c - 'a' + 26;
		if (c >= '0' && c <= '9') return c - '0' + 52;
		if (c == '+') return 62;
		if (c == '/') return 63;
		return -1;
	}

	bool v_is_leap(int year) noexcept
	{
		return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
	}
}

bool sp::validate::is_iban(const cppString& s) noexcept
{
	String compact;
	compact.reserve(s.s_str().size());

	for (char c : s.s_str())
	{
		if (c == ' ')
			continue;

		if (v_is_lower(c))
			c = static_cast<char>(c - 'a' + 'A');

		compact += c;
	}

	if (compact.size() < 15 || compact.size() > 34)
		return false;

	if (!v_is_upper(compact[0]) || !v_is_upper(compact[1]))
		return false;

	if (!v_is_digit(compact[2]) || !v_is_digit(compact[3]))
		return false;

	for (std::size_t i = 4; i < compact.size(); ++i)
	{
		if (!v_is_alnum(compact[i]))
			return false;
	}

	// Move the country code and check digits to the end, expand letters
	// (A=10 .. Z=35) and verify the mod-97 remainder equals 1.
	const String rearranged = compact.substr(4) + compact.substr(0, 4);
	int remainder = 0;

	for (const char c : rearranged)
	{
		if (v_is_digit(c))
			remainder = (remainder * 10 + (c - '0')) % 97;
		else
			remainder = (remainder * 100 + (c - 'A' + 10)) % 97;
	}

	return remainder == 1;
}

bool sp::validate::is_jwt(const cppString& s) noexcept
{
	const String& src = s.s_str();
	std::size_t segments = 0;
	std::size_t start = 0;

	while (true)
	{
		const std::size_t dot = src.find('.', start);
		const std::size_t end = dot == String::npos ? src.size() : dot;

		if (end == start)
			return false;

		for (std::size_t i = start; i < end; ++i)
		{
			if (!v_is_base64url(src[i]))
				return false;
		}

		++segments;

		if (dot == String::npos)
			break;

		start = dot + 1;
	}

	return segments == 3;
}

bool sp::validate::is_base64(const cppString& s) noexcept
{
	const String& src = s.s_str();

	if (src.empty() || src.size() % 4 != 0)
		return false;

	std::size_t padding = 0;

	while (padding < 2 && src[src.size() - 1 - padding] == '=')
		++padding;

	const std::size_t body = src.size() - padding;

	if (body == 0)
		return false;

	for (std::size_t i = 0; i < body; ++i)
	{
		if (v_base64_value(src[i]) < 0)
			return false;
	}

	if (padding != 0)
	{
		const int last = v_base64_value(src[body - 1]);
		const int unused_bits = (1 << (padding * 2)) - 1;

		if ((last & unused_bits) != 0)
			return false;
	}

	return true;
}

bool sp::validate::is_hostname(const cppString& s) noexcept
{
	String text = s.s_str();

	if (text.empty())
		return false;

	if (text.back() == '.')
		text.pop_back();

	if (text.empty() || text.size() > 253)
		return false;

	std::size_t start = 0;

	while (true)
	{
		const std::size_t dot = text.find('.', start);
		const std::size_t end = dot == String::npos ? text.size() : dot;
		const std::size_t length = end - start;

		if (length == 0 || length > 63)
			return false;

		for (std::size_t i = start; i < end; ++i)
		{
			if (!v_is_alnum(text[i]) && text[i] != '-')
				return false;
		}

		if (text[start] == '-' || text[end - 1] == '-')
			return false;

		if (dot == String::npos)
			break;

		start = dot + 1;
	}

	return true;
}

bool sp::validate::is_json(const cppString& s) noexcept
{
	try
	{
		(void)sp::json::parse(s);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool sp::validate::is_date(const cppString& s) noexcept
{
	const String& src = s.s_str();

	if (src.size() != 10)
		return false;

	for (std::size_t i = 0; i < src.size(); ++i)
	{
		if (i == 4 || i == 7)
		{
			if (src[i] != '-')
				return false;
		}
		else if (!v_is_digit(src[i]))
		{
			return false;
		}
	}

	const int year = (src[0] - '0') * 1000 + (src[1] - '0') * 100
		+ (src[2] - '0') * 10 + (src[3] - '0');
	const int month = (src[5] - '0') * 10 + (src[6] - '0');
	const int day = (src[8] - '0') * 10 + (src[9] - '0');

	if (month < 1 || month > 12)
		return false;

	static const int days_in_month[] =
	{
		0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	const int maximum = (month == 2 && v_is_leap(year)) ? 29 : days_in_month[month];

	return day >= 1 && day <= maximum;
}


bool sp::validate::is_slug(const cppString& s) noexcept
{
	const String& v = s.s_str();
	if (v.empty())
		return false;

	bool hyphen = true;   // a leading '-' is rejected on the first iteration
	for (const char c : v)
	{
		if (c == '-')
		{
			if (hyphen)
				return false;
			hyphen = true;
		}
		else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
		{
			hyphen = false;
		}
		else
		{
			return false;
		}
	}
	return !hyphen;
}

bool sp::validate::is_ipv4_cidr(const cppString& s) noexcept
{
	const String& v = s.s_str();
	const std::size_t slash = v.find('/');
	if (slash == String::npos)
		return false;
	if (!is_ipv4(cppString{ v.substr(0, slash) }))
		return false;

	const String prefix = v.substr(slash + 1);
	if (prefix.empty() || prefix.size() > 2)
		return false;
	int value = 0;
	for (const char c : prefix)
	{
		if (c < '0' || c > '9')
			return false;
		value = value * 10 + (c - '0');
	}
	return value <= 32;
}
