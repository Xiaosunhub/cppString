// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "algorithm.h"
#include "utf8.h"
#include <algorithm>
#include <array>
#include <cctype>

namespace cppstr::algorithm
{
	namespace
	{
		// KMP failure function: for each prefix length, the length of its
		// longest proper prefix that is also a suffix.
		std::vector<std::size_t> kmp_failure(const String& needle)
		{
			std::vector<std::size_t> failure(needle.size(), 0);

			std::size_t k = 0;
			for (std::size_t i = 1; i < needle.size(); ++i)
			{
				while (k > 0 && needle[i] != needle[k])
					k = failure[k - 1];

				if (needle[i] == needle[k])
					++k;

				failure[i] = k;
			}

			return failure;
		}
	}

	// ---- substring search ----

	std::size_t kmp_find(const cppString& haystack, const cppString& needle)
	{
		const String& h = haystack.s_str();
		const String& n = needle.s_str();

		if (n.empty())
			return 0;

		if (n.size() > h.size())
			return cppString::npos;

		const std::vector<std::size_t> failure = kmp_failure(n);

		std::size_t k = 0;
		for (std::size_t i = 0; i < h.size(); ++i)
		{
			while (k > 0 && h[i] != n[k])
				k = failure[k - 1];

			if (h[i] == n[k])
				++k;

			if (k == n.size())
				return i + 1 - n.size();
		}

		return cppString::npos;
	}

	std::vector<std::size_t> kmp_find_all(const cppString& haystack,
		const cppString& needle, bool overlapping)
	{
		std::vector<std::size_t> hits;

		const String& h = haystack.s_str();
		const String& n = needle.s_str();

		if (n.empty() || n.size() > h.size())
			return hits;

		const std::vector<std::size_t> failure = kmp_failure(n);

		std::size_t k = 0;
		for (std::size_t i = 0; i < h.size(); ++i)
		{
			while (k > 0 && h[i] != n[k])
				k = failure[k - 1];

			if (h[i] == n[k])
				++k;

			if (k == n.size())
			{
				hits.push_back(i + 1 - n.size());
				k = overlapping ? failure[k - 1] : 0;
			}
		}

		return hits;
	}

	std::size_t rabin_karp_find(const cppString& haystack, const cppString& needle)
	{
		const String& h = haystack.s_str();
		const String& n = needle.s_str();

		if (n.empty())
			return 0;

		if (n.size() > h.size())
			return cppString::npos;

		// Fixed base and modulus; every candidate is verified byte by byte,
		// so collisions are only a performance concern, never correctness.
		constexpr unsigned long long modulus = 1000000007ULL;
		constexpr unsigned long long base = 131ULL;

		unsigned long long hash_n = 0;
		unsigned long long hash_h = 0;
		unsigned long long power = 1; // base^(m-1) mod modulus

		for (std::size_t i = 0; i < n.size(); ++i)
		{
			hash_n = (hash_n * base + static_cast<unsigned char>(n[i])) % modulus;
			hash_h = (hash_h * base + static_cast<unsigned char>(h[i])) % modulus;
			if (i + 1 < n.size())
				power = (power * base) % modulus;
		}

		for (std::size_t start = 0; start + n.size() <= h.size(); ++start)
		{
			if (hash_h == hash_n)
			{
				if (h.compare(start, n.size(), n) == 0)
					return start;
			}

			if (start + n.size() < h.size())
			{
				// Remove the leading byte, append the next one.
				hash_h = (hash_h + modulus
					- (power * static_cast<unsigned char>(h[start])) % modulus)
					% modulus;
				hash_h = (hash_h * base
					+ static_cast<unsigned char>(h[start + n.size()])) % modulus;
			}
		}

		return cppString::npos;
	}

	// ---- permutations ----

	bool next_permutation(cppString& s)
	{
		String value = s.s_str();

		if (value.empty())
			return false;

		// Walk back to the first ascent.
		std::size_t i = value.size() - 1;
		while (i > 0 && value[i - 1] >= value[i])
			--i;

		const bool has_ascent = i > 0;

		if (has_ascent)
		{
			// Smallest suffix byte strictly greater than the pivot.
			std::size_t j = value.size() - 1;
			while (value[j] <= value[i - 1])
				--j;

			std::swap(value[i - 1], value[j]);
		}

		std::reverse(value.begin() + static_cast<std::ptrdiff_t>(i), value.end());
		s = cppString(value);
		return has_ascent;
	}

	bool prev_permutation(cppString& s)
	{
		String value = s.s_str();

		if (value.empty())
			return false;

		// Walk back to the first descent.
		std::size_t i = value.size() - 1;
		while (i > 0 && value[i - 1] <= value[i])
			--i;

		const bool has_descent = i > 0;

		if (has_descent)
		{
			// Largest suffix byte strictly smaller than the pivot.
			std::size_t j = value.size() - 1;
			while (value[j] >= value[i - 1])
				--j;

			std::swap(value[i - 1], value[j]);
		}

		std::reverse(value.begin() + static_cast<std::ptrdiff_t>(i), value.end());
		s = cppString(value);
		return has_descent;
	}

	bool is_permutation_of(const cppString& a, const cppString& b) noexcept
	{
		if (a.size() != b.size())
			return false;

		std::array<std::size_t, 256> counts{};
		for (unsigned char c : a.s_str())
			++counts[c];
		for (unsigned char c : b.s_str())
			--counts[c];
		for (std::size_t c = 0; c < 256; ++c)
		{
			if (counts[c] != 0)
				return false;
		}

		return true;
	}

	// ---- common sequence helpers (UTF-8 aware) ----

	cppString longest_common_subsequence(const cppString& a, const cppString& b)
	{
		if (!utf8::is_valid(a))
			throw Invalid_Format("longest_common_subsequence: input is not valid UTF-8.");
		if (!utf8::is_valid(b))
			throw Invalid_Format("longest_common_subsequence: input is not valid UTF-8.");

		const std::vector<char32_t> x = utf8::codepoints(a);
		const std::vector<char32_t> y = utf8::codepoints(b);

		// Full table so the sequence itself can be reconstructed.
		std::vector<std::vector<std::size_t>> table(
			x.size() + 1, std::vector<std::size_t>(y.size() + 1, 0));

		for (std::size_t i = 1; i <= x.size(); ++i)
		{
			for (std::size_t j = 1; j <= y.size(); ++j)
			{
				if (x[i - 1] == y[j - 1])
					table[i][j] = table[i - 1][j - 1] + 1;
				else
					table[i][j] = std::max(table[i - 1][j], table[i][j - 1]);
			}
		}

		// Walk back through the table.
		std::vector<char32_t> reversed;
		std::size_t i = x.size();
		std::size_t j = y.size();
		while (i > 0 && j > 0)
		{
			if (x[i - 1] == y[j - 1])
			{
				reversed.push_back(x[i - 1]);
				--i;
				--j;
			}
			else if (table[i - 1][j] >= table[i][j - 1])
			{
				--i;
			}
			else
			{
				--j;
			}
		}

		String result;
		for (auto it = reversed.rbegin(); it != reversed.rend(); ++it)
			result += utf8::from_codepoint(*it).s_str();

		return cppString(result);
	}

	namespace
	{
		// Shared code point count of the common prefix/suffix across all
		// inputs; the caller converts it to a byte substring of strings[0].
		std::size_t common_run(const std::vector<cppString>& strings, bool suffix)
		{
			if (strings.empty())
				return 0;

			if (strings.size() == 1)
				return utf8::length(strings[0]);

			for (const cppString& s : strings)
			{
				if (!utf8::is_valid(s))
					throw Invalid_Format(
						suffix ? "longest_common_suffix: input is not valid UTF-8."
						       : "longest_common_prefix: input is not valid UTF-8.");
			}

			const std::vector<char32_t> first =
				utf8::codepoints(strings[0]);
			std::size_t limit = first.size();

			for (std::size_t s = 1; s < strings.size() && limit > 0; ++s)
			{
				const std::vector<char32_t> other =
					utf8::codepoints(strings[s]);
				const std::size_t other_size = other.size();
				if (other_size < limit)
					limit = other_size;

				for (std::size_t k = 0; k < limit; ++k)
				{
					const char32_t left = suffix
						? first[first.size() - 1 - k]
						: first[k];
					const char32_t right = suffix
						? other[other_size - 1 - k]
						: other[k];

					if (left != right)
					{
						limit = k;
						break;
					}
				}
			}

			return limit;
		}
	}

	cppString longest_common_prefix(const std::vector<cppString>& strings)
	{
		if (strings.empty())
			return cppString();

		const std::size_t count = common_run(strings, false);
		return utf8::sub(strings[0], 0, count);
	}

	cppString longest_common_suffix(const std::vector<cppString>& strings)
	{
		if (strings.empty())
			return cppString();

		const std::size_t count = common_run(strings, true);
		const std::size_t total = utf8::length(strings[0]);
		return utf8::sub(strings[0], total - count, count);
	}

	cppString longest_unique_run(const cppString& s)
	{
		const String& value = s.s_str();

		std::array<std::size_t, 256> last_seen{};
		last_seen.fill(cppString::npos);

		std::size_t best_start = 0;
		std::size_t best_length = 0;
		std::size_t window_start = 0;

		for (std::size_t i = 0; i < value.size(); ++i)
		{
			const unsigned char c = static_cast<unsigned char>(value[i]);

			if (last_seen[c] != cppString::npos && last_seen[c] >= window_start)
				window_start = last_seen[c] + 1;

			last_seen[c] = i;

			if (i - window_start + 1 > best_length)
			{
				best_length = i - window_start + 1;
				best_start = window_start;
			}
		}

		return s.substr(best_start, best_length);
	}

	// ---- line-oriented text operations ----

	namespace
	{
		// Split on '\n' (keeping content before each break); reports whether
		// the text ended with a newline so it can be re-added on join.
		std::vector<String> split_lines(const String& text, bool& trailing_newline)
		{
			std::vector<String> lines;
			trailing_newline = false;

			String current;
			for (const char c : text)
			{
				if (c == '\n')
				{
					lines.push_back(current);
					current.clear();
					trailing_newline = true;
				}
				else
				{
					current.push_back(c);
				}
			}

			if (!current.empty() || !trailing_newline)
				lines.push_back(current);

			// "a\n" -> one line + trailing newline; "" -> one empty line.
			return lines;
		}

		bool line_less(const String& a, const String& b,
			const SortLinesOptions& options)
		{
			if (options.case_insensitive)
			{
				for (std::size_t k = 0; k < a.size() && k < b.size(); ++k)
				{
					const int left = std::tolower(
						static_cast<unsigned char>(a[k]));
					const int right = std::tolower(
						static_cast<unsigned char>(b[k]));
					if (left != right)
						return left < right;
				}

				if (a.size() != b.size())
					return a.size() < b.size();

				return false;
			}

			return a < b;
		}
	}

	cppString sort_lines(const cppString& text, const SortLinesOptions& options)
	{
		bool trailing_newline = false;
		std::vector<String> lines = split_lines(text.s_str(), trailing_newline);

		if (options.descending)
		{
			std::stable_sort(lines.begin(), lines.end(),
				[&options](const String& a, const String& b)
				{
					return line_less(b, a, options);
				});
		}
		else
		{
			std::stable_sort(lines.begin(), lines.end(),
				[&options](const String& a, const String& b)
				{
					return line_less(a, b, options);
				});
		}

		if (options.unique)
			lines.erase(std::unique(lines.begin(), lines.end()), lines.end());

		String result;
		for (std::size_t i = 0; i < lines.size(); ++i)
		{
			if (i > 0)
				result.push_back('\n');
			result += lines[i];
		}
		if (trailing_newline)
			result.push_back('\n');

		return cppString(result);
	}

	cppString rotate(const cppString& s, std::size_t n)
	{
		const String& value = s.s_str();

		if (value.empty())
			return cppString();

		n %= value.size();

		return cppString(value.substr(n) + value.substr(0, n));
	}

	// ---- classic reductions ----

	cppString to_roman(unsigned value)
	{
		if (value == 0 || value > 3999)
			throw Invalid_Argument("to_roman: value must be in 1..3999.");

		static constexpr std::array<std::pair<unsigned, const char*>, 13> table{{
			{ 1000, "M" }, { 900, "CM" }, { 500, "D" }, { 400, "CD" },
			{ 100, "C" }, { 90, "XC" }, { 50, "L" }, { 40, "XL" },
			{ 10, "X" }, { 9, "IX" }, { 5, "V" }, { 4, "IV" }, { 1, "I" },
		}};

		String result;
		for (const auto& [threshold, numeral] : table)
		{
			while (value >= threshold)
			{
				result += numeral;
				value -= threshold;
			}
		}

		return cppString(result);
	}

	namespace
	{
		constexpr std::array<std::pair<char, unsigned>, 7> roman_values{{
			{ 'I', 1 }, { 'V', 5 }, { 'X', 10 }, { 'L', 50 },
			{ 'C', 100 }, { 'D', 500 }, { 'M', 1000 },
		}};

		int roman_value_of(char c) noexcept
		{
			for (const auto& [symbol, value] : roman_values)
			{
				if (symbol == c)
					return static_cast<int>(value);
			}
			return -1;
		}
	}

	unsigned from_roman(const cppString& numeral)
	{
		const String text = numeral.s_str();
		if (text.empty())
			throw Invalid_Format("from_roman: numeral is empty.");

		// Walk right to left: a symbol smaller than the one to its right is
		// subtractive; everything else adds.
		unsigned result = 0;
		int previous = 0;

		for (auto it = text.rbegin(); it != text.rend(); ++it)
		{
			const int current = roman_value_of(*it);
			if (current < 0)
				throw Invalid_Format("from_roman: invalid numeral character.");

			if (current < previous)
				result -= static_cast<unsigned>(current);
			else
				result += static_cast<unsigned>(current);

			previous = current;
		}

		if (result == 0 || result > 3999)
			throw Invalid_Format("from_roman: numeral out of range 1..3999.");

		return result;
	}

	cppString run_length_encode(const cppString& s)
	{
		const String& value = s.s_str();

		String result;
		for (std::size_t i = 0; i < value.size();)
		{
			const char c = value[i];
			std::size_t run = 1;
			while (i + run < value.size() && value[i + run] == c)
				++run;

			result.push_back(c);
			result += std::to_string(run);
			i += run;
		}

		return cppString(result);
	}

	cppString run_length_decode(const cppString& s)
	{
		const String& value = s.s_str();

		String result;
		for (std::size_t i = 0; i < value.size();)
		{
			const char c = value[i];

			if (c >= '0' && c <= '9')
				throw Invalid_Format(
					"run_length_decode: count without a preceding character.");

			++i;

			if (i >= value.size() || value[i] < '0' || value[i] > '9')
				throw Invalid_Format(
					"run_length_decode: character without a count.");

			unsigned long long count = 0;
			while (i < value.size() && value[i] >= '0' && value[i] <= '9')
			{
				const unsigned long long digit =
					static_cast<unsigned long long>(value[i] - '0');
				if (count > (result.max_size() - digit) / 10)
					throw Invalid_Format(
						"run_length_decode: run count is too large.");
				count = count * 10 + digit;
				++i;
			}

			if (count == 0)
				throw Invalid_Format("run_length_decode: zero run count.");

			result.append(static_cast<std::size_t>(count), c);
		}

		return cppString(result);
	}

	bool majority_byte(const cppString& s, char& out)
	{
		const String& value = s.s_str();

		if (value.empty())
			return false;

		// Boyer-Moore vote.
		char candidate = value[0];
		std::size_t votes = 1;
		for (std::size_t i = 1; i < value.size(); ++i)
		{
			if (value[i] == candidate)
				++votes;
			else if (--votes == 0)
			{
				candidate = value[i];
				votes = 1;
			}
		}

		// Second pass: the vote only guarantees a candidate, verify it.
		const std::size_t occurrences =
			static_cast<std::size_t>(std::count(value.begin(), value.end(),
				candidate));
		if (occurrences * 2 <= value.size())
			return false;

		out = candidate;
		return true;
	}
}
