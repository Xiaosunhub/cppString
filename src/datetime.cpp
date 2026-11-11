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

#include "datetime.h"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <string>
#include <utility>

namespace sp
{
	namespace datetime
	{
		namespace
		{
			constexpr const char* WEEK_SHORT[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
			constexpr const char* WEEK_LONG[7] = {
				"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
			constexpr const char* MONTH_SHORT[12] = {
				"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
			constexpr const char* MONTH_LONG[12] = {
				"January", "February", "March", "April", "May", "June",
				"July", "August", "September", "October", "November", "December" };
			constexpr unsigned MONTH_DAYS[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

			bool is_digit(char c) noexcept { return c >= '0' && c <= '9'; }

			std::int64_t floor_div(std::int64_t a, std::int64_t b) noexcept
			{
				const std::int64_t q = a / b;
				const std::int64_t r = a % b;
				return (r != 0 && ((r < 0) != (b < 0))) ? q - 1 : q;
			}

			void append_number(String& out, long long value, int width)
			{
				char buffer[64]{};
				if (value < 0)
					std::snprintf(buffer, sizeof(buffer), "-%0*lld", width - 1, -value);
				else
					std::snprintf(buffer, sizeof(buffer), "%0*lld", width, value);
				out += buffer;
			}

			bool read_digits(const String& s, std::size_t& pos, std::size_t count, int& out) noexcept
			{
				if (pos + count > s.size())
					return false;
				int value = 0;
				for (std::size_t i = 0; i < count; ++i)
				{
					const char c = s[pos + i];
					if (!is_digit(c))
						return false;
					value = value * 10 + (c - '0');
				}
				pos += count;
				out = value;
				return true;
			}

			std::int64_t to_unix_raw(const DateTime& dt) noexcept
			{
				const std::int64_t days = days_from_civil(dt.year, dt.month, dt.day);
				return days * 86400
					+ static_cast<std::int64_t>(dt.hour) * 3600
					+ static_cast<std::int64_t>(dt.minute) * 60
					+ static_cast<std::int64_t>(dt.second)
					- static_cast<std::int64_t>(dt.offset_minutes) * 60;
			}

			void require_valid(const DateTime& dt, const char* where)
			{
				if (!is_valid(dt))
					throw sp::Invalid_Conversion(String{ where } + ": invalid date/time");
			}

			cppString offset_suffix(const DateTime& dt, bool force)
			{
				String out{};
				if (!dt.has_offset)
				{
					if (force)
						out += 'Z';       // unspecified timezone is treated as UTC
				}
				else if (dt.offset_minutes == 0)
				{
					out += 'Z';
				}
				else
				{
					const int abs = dt.offset_minutes < 0 ? -dt.offset_minutes : dt.offset_minutes;
					out += dt.offset_minutes < 0 ? '-' : '+';
					append_number(out, abs / 60, 2);
					out += ':';
					append_number(out, abs % 60, 2);
				}
				return cppString{ std::move(out) };
			}
		}

		bool is_leap_year(int year) noexcept
		{
			return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
		}

		unsigned days_in_month(int year, unsigned month) noexcept
		{
			if (month < 1 || month > 12)
				return 0;
			if (month == 2 && is_leap_year(year))
				return 29;
			return MONTH_DAYS[month - 1];
		}

		bool is_valid(const DateTime& dt) noexcept
		{
			if (dt.month < 1 || dt.month > 12)
				return false;
			if (dt.day < 1 || dt.day > days_in_month(dt.year, dt.month))
				return false;
			if (dt.hour > 23 || dt.minute > 59 || dt.second > 59 || dt.millisecond > 999)
				return false;
			return dt.offset_minutes >= -14 * 60 && dt.offset_minutes <= 14 * 60;
		}

		namespace
		{
			// A Gregorian 400-year cycle is always 146097 days long, and
			// 2000-01-01 sits 10957 days after 1970-01-01 (30*365 + 7 leap
			// days). These two facts are the whole basis of the conversion:
			// shift the year into one cycle by whole cycles, then count the
			// remaining years/months directly from the leap rule. This is an
			// independent implementation; no third-party date algorithm,
			// source code or lookup table is reused.
			constexpr std::int64_t kCycleDays = 146097;
			constexpr std::int64_t kAnchorDays = 10957;
			constexpr unsigned kDaysBeforeMonth[12] = {
				0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
			constexpr unsigned kMonthLength[12] = {
				31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		}

		std::int64_t days_from_civil(int year, unsigned month, unsigned day) noexcept
		{
			if (month < 1 || month > 12)
				return 0;

			std::int64_t cycle_shift = 0;
			while (year < 2000) { year += 400; cycle_shift -= kCycleDays; }
			while (year >= 2400) { year -= 400; cycle_shift += kCycleDays; }

			std::int64_t total = kAnchorDays + cycle_shift;
			for (int y = 2000; y < year; ++y)
				total += is_leap_year(y) ? 366 : 365;

			total += kDaysBeforeMonth[month - 1];
			if (month > 2 && is_leap_year(year))
				++total;
			return total + static_cast<std::int64_t>(day) - 1;
		}

		DateTime civil_from_days(std::int64_t days) noexcept
		{
			const std::int64_t shifted = days - kAnchorDays;
			const std::int64_t cycles = floor_div(shifted, kCycleDays);
			std::int64_t remaining = shifted - cycles * kCycleDays;
			int year = static_cast<int>(2000 + cycles * 400);

			for (;;)
			{
				const std::int64_t length = is_leap_year(year) ? 366 : 365;
				if (remaining < length)
					break;
				remaining -= length;
				++year;
			}

			unsigned month = 1;
			for (; month <= 12; ++month)
			{
				std::int64_t length = kMonthLength[month - 1];
				if (month == 2 && is_leap_year(year))
					++length;
				if (remaining < length)
					break;
				remaining -= length;
			}

			DateTime dt{};
			dt.year = year;
			dt.month = month;
			dt.day = static_cast<unsigned>(remaining + 1);
			return dt;
		}

		std::int64_t to_unix(const DateTime& dt)
		{
			require_valid(dt, "sp::datetime::to_unix");
			return to_unix_raw(dt);
		}

		DateTime from_unix(std::int64_t seconds, int offset_minutes)
		{
			if (offset_minutes < -14 * 60 || offset_minutes > 14 * 60)
				throw sp::Invalid_Argument("sp::datetime::from_unix: UTC offset out of range");

			const std::int64_t local = seconds + static_cast<std::int64_t>(offset_minutes) * 60;
			const std::int64_t days = floor_div(local, 86400);
			std::int64_t sod = local - days * 86400;

			DateTime dt = civil_from_days(days);
			dt.hour = static_cast<unsigned>(sod / 3600);
			sod %= 3600;
			dt.minute = static_cast<unsigned>(sod / 60);
			dt.second = static_cast<unsigned>(sod % 60);
			dt.offset_minutes = offset_minutes;
			dt.has_offset = true;
			return dt;
		}

		std::optional<DateTime> parse(const cppString& text)
		{
			const String& s = text.s_str();
			std::size_t p = 0;
			int v = 0;
			DateTime dt{};

			if (!read_digits(s, p, 4, v)) return std::nullopt;
			dt.year = v;
			if (p >= s.size() || s[p] != '-') return std::nullopt;
			++p;
			if (!read_digits(s, p, 2, v)) return std::nullopt;
			dt.month = static_cast<unsigned>(v);
			if (p >= s.size() || s[p] != '-') return std::nullopt;
			++p;
			if (!read_digits(s, p, 2, v)) return std::nullopt;
			dt.day = static_cast<unsigned>(v);

			if (p < s.size())
			{
				const char sep = s[p];
				if (sep != 'T' && sep != 't' && sep != ' ')
					return std::nullopt;
				++p;

				if (!read_digits(s, p, 2, v)) return std::nullopt;
				dt.hour = static_cast<unsigned>(v);
				if (p >= s.size() || s[p] != ':') return std::nullopt;
				++p;
				if (!read_digits(s, p, 2, v)) return std::nullopt;
				dt.minute = static_cast<unsigned>(v);
				if (p >= s.size() || s[p] != ':') return std::nullopt;
				++p;
				if (!read_digits(s, p, 2, v)) return std::nullopt;
				dt.second = static_cast<unsigned>(v);

				if (p < s.size() && (s[p] == '.' || s[p] == ','))
				{
					++p;
					const std::size_t start = p;
					while (p < s.size() && is_digit(s[p]))
						++p;
					if (p == start)
						return std::nullopt;
					int ms = 0;
					for (std::size_t i = 0; i < 3; ++i)
						ms = ms * 10 + ((start + i < p) ? (s[start + i] - '0') : 0);
					dt.millisecond = static_cast<unsigned>(ms);
				}

				if (p < s.size())
				{
					const char z = s[p];
					if (z == 'Z' || z == 'z')
					{
						dt.has_offset = true;
						dt.offset_minutes = 0;
						++p;
					}
					else if (z == '+' || z == '-')
					{
						const int sign = (z == '-') ? -1 : 1;
						++p;
						if (!read_digits(s, p, 2, v)) return std::nullopt;
						const int oh = v;
						int om = 0;
						if (p < s.size() && s[p] == ':')
						{
							++p;
							if (!read_digits(s, p, 2, v)) return std::nullopt;
							om = v;
						}
						else if (p + 2 <= s.size() && is_digit(s[p]) && is_digit(s[p + 1]))
						{
							if (!read_digits(s, p, 2, v)) return std::nullopt;
							om = v;
						}
						if (oh > 23 || om > 59)
							return std::nullopt;
						dt.has_offset = true;
						dt.offset_minutes = sign * (oh * 60 + om);
					}
				}
			}

			if (p != s.size() || !is_valid(dt))
				return std::nullopt;
			return dt;
		}

		cppString format(const DateTime& dt, const cppString& pattern)
		{
			require_valid(dt, "sp::datetime::format");
			const String& p = pattern.s_str();
			const int wd = weekday(dt);
			String out{};
			out.reserve(p.size() + 16);

			for (std::size_t i = 0; i < p.size();)
			{
				if (p[i] != '%' || i + 1 == p.size())
				{
					out += p[i++];
					continue;
				}

				const char c = p[i + 1];
				i += 2;
				switch (c)
				{
				case 'Y': append_number(out, dt.year, 4); break;
				case 'y': append_number(out, ((dt.year % 100) + 100) % 100, 2); break;
				case 'm': append_number(out, dt.month, 2); break;
				case 'd': append_number(out, dt.day, 2); break;
				case 'H': append_number(out, dt.hour, 2); break;
				case 'I': append_number(out, (dt.hour % 12 == 0) ? 12 : (dt.hour % 12), 2); break;
				case 'M': append_number(out, dt.minute, 2); break;
				case 'S': append_number(out, dt.second, 2); break;
				case 'f': append_number(out, dt.millisecond, 3); break;
				case 'p': out += (dt.hour < 12) ? "AM" : "PM"; break;
				case 'j': append_number(out, day_of_year(dt), 3); break;
				case 'a': out += WEEK_SHORT[wd]; break;
				case 'A': out += WEEK_LONG[wd]; break;
				case 'b': out += MONTH_SHORT[dt.month - 1]; break;
				case 'B': out += MONTH_LONG[dt.month - 1]; break;
				case 'z': out += offset_suffix(dt, false).s_str(); break;
				case '%': out += '%'; break;
				default:  out += '%'; out += c; break;
				}
			}

			return cppString{ std::move(out) };
		}

		cppString to_iso8601(const DateTime& dt)
		{
			require_valid(dt, "sp::datetime::to_iso8601");
			const cppString base = format(dt, "%Y-%m-%dT%H:%M:%S");
			String out = base.s_str();
			if (dt.millisecond != 0)
			{
				out += '.';
				append_number(out, dt.millisecond, 3);
			}
			out += offset_suffix(dt, false).s_str();
			return cppString{ std::move(out) };
		}

		cppString to_rfc3339(const DateTime& dt)
		{
			require_valid(dt, "sp::datetime::to_rfc3339");
			const cppString base = format(dt, "%Y-%m-%dT%H:%M:%S");
			String out = base.s_str();
			if (dt.millisecond != 0)
			{
				out += '.';
				append_number(out, dt.millisecond, 3);
			}
			out += offset_suffix(dt, true).s_str();
			return cppString{ std::move(out) };
		}

		int weekday(const DateTime& dt) noexcept
		{
			const std::int64_t days = days_from_civil(dt.year, dt.month, dt.day);
			int wd = static_cast<int>((days + 4) % 7);
			if (wd < 0)
				wd += 7;
			return wd;
		}

		unsigned day_of_year(const DateTime& dt) noexcept
		{
			return static_cast<unsigned>(
				days_from_civil(dt.year, dt.month, dt.day) - days_from_civil(dt.year, 1, 1) + 1);
		}

		DateTime add_days(DateTime dt, int days)
		{
			require_valid(dt, "sp::datetime::add_days");
			DateTime out = civil_from_days(days_from_civil(dt.year, dt.month, dt.day) + days);
			out.hour = dt.hour;
			out.minute = dt.minute;
			out.second = dt.second;
			out.millisecond = dt.millisecond;
			out.offset_minutes = dt.offset_minutes;
			out.has_offset = dt.has_offset;
			return out;
		}

		DateTime add_seconds(DateTime dt, std::int64_t seconds)
		{
			DateTime out = from_unix(to_unix(dt) + seconds, dt.offset_minutes);
			out.millisecond = dt.millisecond;
			out.has_offset = dt.has_offset;
			return out;
		}

		std::int64_t diff_seconds(const DateTime& a, const DateTime& b) noexcept
		{
			return to_unix_raw(a) - to_unix_raw(b);
		}

		int diff_days(const DateTime& a, const DateTime& b) noexcept
		{
			return static_cast<int>(
				days_from_civil(a.year, a.month, a.day) - days_from_civil(b.year, b.month, b.day));
		}

		std::int64_t unix_now() noexcept
		{
			return std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
		}

		DateTime utc_now()
		{
			const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
			const std::int64_t sec = ms / 1000;
			DateTime dt = from_unix(sec, 0);
			dt.millisecond = static_cast<unsigned>(ms - sec * 1000);
			return dt;
		}

		DateTime local_now()
		{
			const std::time_t t = static_cast<std::time_t>(unix_now());
			std::tm local{};
#ifdef _WIN32
			localtime_s(&local, &t);
#else
			localtime_r(&t, &local);
#endif
			DateTime dt{};
			dt.year = local.tm_year + 1900;
			dt.month = static_cast<unsigned>(local.tm_mon + 1);
			dt.day = static_cast<unsigned>(local.tm_mday);
			dt.hour = static_cast<unsigned>(local.tm_hour);
			dt.minute = static_cast<unsigned>(local.tm_min);
			dt.second = static_cast<unsigned>(local.tm_sec);

			const std::int64_t as_utc = days_from_civil(dt.year, dt.month, dt.day) * 86400
				+ static_cast<std::int64_t>(dt.hour) * 3600
				+ static_cast<std::int64_t>(dt.minute) * 60
				+ static_cast<std::int64_t>(dt.second);
			dt.offset_minutes = static_cast<int>((as_utc - static_cast<std::int64_t>(t)) / 60);
			dt.has_offset = true;
			return dt;
		}

		cppString relative(std::int64_t timestamp, std::int64_t now)
		{
			// Subtract as unsigned: a signed "now - timestamp" can overflow
			// for extreme inputs, and negating INT64_MIN is undefined.
			const bool future = now < timestamp;
			const std::uint64_t magnitude = future
				? static_cast<std::uint64_t>(timestamp) - static_cast<std::uint64_t>(now)
				: static_cast<std::uint64_t>(now) - static_cast<std::uint64_t>(timestamp);

			auto phrase = [&](std::uint64_t n, const char* unit) -> cppString
			{
				String s{};
				if (future)
					s += "in ";
				s += std::to_string(n);
				s += ' ';
				s += unit;
				if (n != 1)
					s += 's';
				if (!future)
					s += " ago";
				return cppString{ std::move(s) };
			};

			if (magnitude < 10)  return cppString{ "just now" };
			if (magnitude < 60)  return phrase(magnitude, "second");
			if (magnitude < 3600) return phrase(magnitude / 60, "minute");
			if (magnitude < 86400) return phrase(magnitude / 3600, "hour");
			if (magnitude < 2 * 86400)
				return cppString{ future ? "tomorrow" : "yesterday" };
			if (magnitude < 7 * 86400)   return phrase(magnitude / 86400, "day");
			if (magnitude < 30 * 86400)  return phrase(magnitude / (7 * 86400), "week");
			if (magnitude < 365 * 86400) return phrase(magnitude / (30 * 86400), "month");
			return phrase(magnitude / (365 * 86400), "year");
		}

		cppString duration(std::int64_t seconds)
		{
			const bool negative = seconds < 0;
			// -INT64_MIN overflows; the unsigned difference yields the exact
			// magnitude (2^63) without undefined behaviour.
			std::uint64_t v = negative
				? 0ULL - static_cast<std::uint64_t>(seconds)
				: static_cast<std::uint64_t>(seconds);
			String out{};
			if (negative)
				out += '-';

			const std::uint64_t days = v / 86400;
			v %= 86400;
			const std::uint64_t hours = v / 3600;
			v %= 3600;
			const std::uint64_t minutes = v / 60;
			const std::uint64_t secs = v % 60;

			if (days > 0)
			{
				out += std::to_string(days);
				out += "d ";
			}
			append_number(out, static_cast<long long>(hours), 2);
			out += ':';
			append_number(out, static_cast<long long>(minutes), 2);
			out += ':';
			append_number(out, static_cast<long long>(secs), 2);
			return cppString{ std::move(out) };
		}
	}
}
