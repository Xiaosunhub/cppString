// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "semver.h"
#include <limits>

namespace
{
	bool is_ident_char(char c) noexcept
	{
		const unsigned char uc = static_cast<unsigned char>(c);
		return std::isalnum(uc) || c == '-';
	}

	bool is_numeric(const String& s) noexcept
	{
		return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) {
			return std::isdigit(static_cast<unsigned char>(c)) != 0;
		});
	}

	// "0" or a number without leading zeros.
	bool is_valid_number(const String& s) noexcept
	{
		return is_numeric(s) && (s.size() == 1 || s[0] != '0');
	}

	// Dot-separated identifiers, each [0-9A-Za-z-]+. require no leading zeros
	// on purely numeric identifiers (prerelease) unless allow_leading_zero (build).
	bool valid_idents(const String& s, bool allow_leading_zero) noexcept
	{
		if (s.empty())
			return false;

		std::size_t start = 0;

		while (true)
		{
			const std::size_t dot = s.find('.', start);
			const String ident = s.substr(start,
				dot == String::npos ? String::npos : dot - start);

			if (ident.empty())
				return false;

			if (!std::all_of(ident.begin(), ident.end(), is_ident_char))
				return false;

			if (!allow_leading_zero && is_numeric(ident)
				&& ident.size() > 1 && ident[0] == '0')
			{
				return false;
			}

			if (dot == String::npos)
				break;

			start = dot + 1;
		}

		return true;
	}

	struct Version
	{
		long long major = 0;
		long long minor = 0;
		long long patch = 0;
		std::vector<String> prerelease; // empty when there is none
	};

	// Split off "+build", validate core + prerelease, parse into Version.
	// Returns false when the string is not valid semver.
	bool parse(const String& src, Version& out) noexcept
	{
		String s = src;

		const std::size_t plus = s.find('+');

		if (plus != String::npos)
		{
			if (s.find('+', plus + 1) != String::npos)
				return false;

			if (!valid_idents(s.substr(plus + 1), true))
				return false;

			s = s.substr(0, plus);
		}

		const std::size_t dash = s.find('-');

		if (dash != String::npos)
		{
			if (!valid_idents(s.substr(dash + 1), false))
				return false;

			String pre = s.substr(dash + 1);
			s = s.substr(0, dash);

			// Split prerelease into identifiers.
			std::size_t start = 0;

			while (true)
			{
				const std::size_t dot = pre.find('.', start);
				out.prerelease.push_back(pre.substr(start,
					dot == String::npos ? String::npos : dot - start));

				if (dot == String::npos)
					break;

				start = dot + 1;
			}
		}

		// Core: exactly three dot-separated numbers.
		const std::size_t d1 = s.find('.');

		if (d1 == String::npos)
			return false;

		const std::size_t d2 = s.find('.', d1 + 1);

		if (d2 == String::npos || s.find('.', d2 + 1) != String::npos)
			return false;

		const String major = s.substr(0, d1);
		const String minor = s.substr(d1 + 1, d2 - d1 - 1);
		const String patch = s.substr(d2 + 1);

		if (!is_valid_number(major) || !is_valid_number(minor) || !is_valid_number(patch))
			return false;

		try
		{
			out.major = std::stoll(major);
			out.minor = std::stoll(minor);
			out.patch = std::stoll(patch);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	int compare_idents(const String& a, const String& b) noexcept
	{
		const bool a_num = is_numeric(a);
		const bool b_num = is_numeric(b);

		if (a_num && b_num)
		{
			if (a.size() != b.size())
				return a.size() < b.size() ? -1 : 1;

			return a.compare(b);
		}

		if (a_num) return -1; // numeric identifiers rank below alphanumeric ones
		if (b_num) return 1;

		return a.compare(b);
	}
}

bool sp::semver::is_valid(const cppString& s) noexcept
{
	Version version{};
	return parse(s.s_str(), version);
}

int sp::semver::compare(const cppString& a, const cppString& b)
{
	Version lhs{}, rhs{};

	if (!parse(a.s_str(), lhs))
		throw sp::Invalid_Format("Left operand is not valid semver: " + a.s_str());

	if (!parse(b.s_str(), rhs))
		throw sp::Invalid_Format("Right operand is not valid semver: " + b.s_str());

	if (lhs.major != rhs.major) return lhs.major < rhs.major ? -1 : 1;
	if (lhs.minor != rhs.minor) return lhs.minor < rhs.minor ? -1 : 1;
	if (lhs.patch != rhs.patch) return lhs.patch < rhs.patch ? -1 : 1;

	// A version with a prerelease ranks below the same version without one.
	if (lhs.prerelease.empty() != rhs.prerelease.empty())
		return lhs.prerelease.empty() ? 1 : -1;

	const std::size_t common = (std::min)(lhs.prerelease.size(), rhs.prerelease.size());

	for (std::size_t i = 0; i < common; ++i)
	{
		const int result = compare_idents(lhs.prerelease[i], rhs.prerelease[i]);

		if (result != 0)
			return result < 0 ? -1 : 1;
	}

	if (lhs.prerelease.size() != rhs.prerelease.size())
		return lhs.prerelease.size() < rhs.prerelease.size() ? -1 : 1;

	return 0;
}

namespace
{
	sp::cppString bump(const sp::cppString& s, int component)
	{
		Version v{};

		if (!parse(s.s_str(), v))
			throw sp::Invalid_Format("Not valid semver in bump: " + s.s_str());

		// Parsed components are capped at LLONG_MAX by std::stoll; bumping
		// that value would be a signed overflow (undefined behaviour).
		constexpr long long limit = (std::numeric_limits<long long>::max)();
		if ((component == 0 && v.major == limit)
			|| (component == 1 && v.minor == limit)
			|| (component == 2 && v.patch == limit))
		{
			throw sp::Invalid_Operation("Version component overflow in semver bump.");
		}

		switch (component)
		{
		case 0: ++v.major; v.minor = 0; v.patch = 0; break;
		case 1: ++v.minor; v.patch = 0; break;
		default: ++v.patch; break;
		}

		return sp::cppString(std::to_string(v.major) + "."
			+ std::to_string(v.minor) + "." + std::to_string(v.patch));
	}
}

sp::cppString sp::semver::bump_major(const cppString& s)
{
	return bump(s, 0);
}

sp::cppString sp::semver::bump_minor(const cppString& s)
{
	return bump(s, 1);
}

sp::cppString sp::semver::bump_patch(const cppString& s)
{
	return bump(s, 2);
}
