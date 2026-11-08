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

#ifndef __INC_DATETIME_H__
#define __INC_DATETIME_H__

#include "cppString.h"
#include <cstdint>
#include <optional>

namespace sp
{
	// Timezone-aware date/time helpers on the proleptic Gregorian calendar.
	// No <chrono> time-zone database dependency: conversions use the classic
	// civil-date algorithms, so they work for years before 1970 and beyond
	// 2038 as long as the value fits in int64_t seconds.
	//
	// parse() is total: malformed input returns std::nullopt instead of
	// throwing. Conversion helpers throw Invalid_Conversion for an invalid
	// DateTime, and Invalid_Argument for an out-of-range UTC offset.
	namespace datetime
	{
		struct CPPSTRING_EXPORT DateTime
		{
			int year{ 1970 };
			unsigned month{ 1 };
			unsigned day{ 1 };
			unsigned hour{ 0 };
			unsigned minute{ 0 };
			unsigned second{ 0 };
			unsigned millisecond{ 0 };
			int offset_minutes{ 0 };    // minutes east of UTC: +480 for +08:00
			bool has_offset{ false };   // false = timezone not specified
		};

		[[nodiscard]] bool is_leap_year(int year) noexcept;
		[[nodiscard]] unsigned days_in_month(int year, unsigned month) noexcept;
		[[nodiscard]] bool is_valid(const DateTime& dt) noexcept;

		// Days since 1970-01-01 (negative for earlier dates), and its inverse.
		[[nodiscard]] std::int64_t days_from_civil(int year, unsigned month, unsigned day) noexcept;
		[[nodiscard]] DateTime civil_from_days(std::int64_t days) noexcept;

		[[nodiscard]] std::int64_t to_unix(const DateTime& dt);
		[[nodiscard]] DateTime from_unix(std::int64_t seconds, int offset_minutes = 0);

		// ISO 8601 / RFC 3339: "YYYY-MM-DD", "YYYY-MM-DDTHH:MM:SS[.fff][Z|±HH:MM]".
		// A single space may replace 'T'; "±HHMM" and "±HH" are also accepted.
		[[nodiscard]] std::optional<DateTime> parse(const cppString& text);

		// strftime-like subset: %Y %y %m %d %H %I %M %S %p %f %j %a %A %b %B %z %%.
		// Unknown %-sequences are emitted verbatim.
		[[nodiscard]] cppString format(const DateTime& dt, const cppString& pattern);
		[[nodiscard]] cppString to_iso8601(const DateTime& dt);
		[[nodiscard]] cppString to_rfc3339(const DateTime& dt);

		[[nodiscard]] int weekday(const DateTime& dt) noexcept;        // 0 = Sunday
		[[nodiscard]] unsigned day_of_year(const DateTime& dt) noexcept;

		[[nodiscard]] DateTime add_days(DateTime dt, int days);
		[[nodiscard]] DateTime add_seconds(DateTime dt, std::int64_t seconds);
		[[nodiscard]] std::int64_t diff_seconds(const DateTime& a, const DateTime& b) noexcept;
		[[nodiscard]] int diff_days(const DateTime& a, const DateTime& b) noexcept;

		[[nodiscard]] std::int64_t unix_now() noexcept;
		[[nodiscard]] DateTime utc_now();
		[[nodiscard]] DateTime local_now();

		// "just now" / "3 minutes ago" / "in 2 hours" / "yesterday" ...
		[[nodiscard]] cppString relative(std::int64_t timestamp, std::int64_t now);
		// "1d 02:03:04" (or "02:03:04"), negative values keep a leading '-'.
		[[nodiscard]] cppString duration(std::int64_t seconds);
	}
}

#endif // __INC_DATETIME_H__
