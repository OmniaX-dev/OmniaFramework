/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

	This file is part of OmniaFramework.

	OmniaFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	OmniaFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with OmniaFramework.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <ostd/string/String.hpp>

namespace ostd
{
	// A storable, comparable date/time value with a configurable textual format.
	//
	// Internally a Date is just a count of seconds since 1970-01-01T00:00:00, computed with pure
	// proleptic-Gregorian calendar math (no mktime/timegm, so no dependency on the process's local
	// timezone) - this is what every comparison, sort and +=/-= operates on, independent of
	// whichever format a given Date happens to be displaying itself in.
	//
	// Format strings are a small template language, e.g. "DD.MM.YYYY" or "YYYY-MM-DD HH:Min:SS":
	//   YYYY  4-digit year                 YY   2-digit year (parsed as 1900+YY or 2000+YY; <70 -> 2000s)
	//   MM    2-digit month (01-12)        DD   2-digit day (01-31)
	//   HH    2-digit hour, 0-padded       hh   hour, not padded   (both 24h, 0-23)
	//   Min   2-digit minute, 0-padded     min  minute, not padded (both 0-59)
	//   SS    2-digit second, 0-padded     ss   second, not padded
	// Minute uses the 3-letter Min/min rather than the 2-letter pattern the other fields use,
	// since MM is already month - a 2-letter mm would either collide with it or (if given to
	// minute instead) leave month with no token at all.
	// Anything in the format string that isn't one of the tokens above (any character, run, or
	// separator - '.', '-', '/', '_', ": ", etc.) is taken as literal text, expected verbatim on
	// parse and reproduced verbatim on output.
	//
	// Parsing (operator=, or the string-taking constructor) validates the input strictly against
	// the current format - "19.12.1993" against "DD.MM.YYYY" works, but "12.19.93" doesn't (wrong
	// token widths/separators), and there's no attempt to guess the format from the input. On
	// failure the Date is left at its previous value (or at "now" for the two-argument constructor,
	// both with isValid() going false and a warning logged) rather than throwing.
	class Date : public I_stringeable
	{
		public:
			// Defaults to "now" (the system clock, read as UTC-based epoch seconds - see the class
			// comment on why that's timezone-independent), format "DD.MM.YYYY".
			Date(void);
			// Parses dateString against format immediately. On failure, falls back to "now" and
			// isValid() is false (see the class comment on parse-failure behavior).
			inline Date(const String& dateString, const String& fmt = "DD.MM.YYYY") : Date() { format(fmt); *this = dateString; }
			Date(i32 year, u32 month, u32 day, u32 hour = 0, u32 minute = 0, u32 second = 0);
			static inline Date now(void) { return Date(); }

			// RightInclusive/LeftExclusive are the same value named from either end ((min, max]),
			// likewise LeftInclusive/RightExclusive ([min, max)).
			enum class eRangeType : u8
			{
				Inclusive = 0,
				Exclusive = 1,
				RightInclusive = 2, LeftExclusive = 2,
				LeftInclusive = 3,  RightExclusive = 3,
			};

			// Every Date from min to max (swapped if given in the wrong order), one per step, each
			// already truncated to and displaying in dest_fmt. The step is calendar-aware and taken
			// from the *finest* field dest_fmt's tokens actually include - "DD.MM.YYYY" steps by
			// day, "MM.YYYY" by calendar month (28-31 days, correctly), "YYYY" by calendar year
			// (365/366 days), "HH:Min:SS"-style formats by hour/minute/second. A format with no
			// recognized token at all (pure literal text) falls back to daily.
			// type controls whether the min-bucket and/or max-bucket are included, same convention
			// as a mathematical interval. Pre-seeding every period in a report range (so e.g. a
			// LineGraph x-axis doesn't skip months with no data) is the motivating use case:
			//   for (auto& bucket : Date::range(start, end, "MM.YYYY"))
			//       plotData[bucket] = 0.0;   // then merge in the real totals afterward
			static stdvec<Date> range(const Date& min, const Date& max, const String& dest_fmt, eRangeType type = eRangeType::Inclusive);

			String toString(void) const override;
			inline String get(void) const { return toString(); }

			// Sets/returns a new copy with a different display format; the underlying moment in
			// time is unchanged either way. Re-parses nothing - only affects toString()/get().
			Date& format(const String& fmt);
			Date new_format(const String& fmt) const;
			inline String getFormat(void) const { return m_format; }

			// Collapses to whatever precision the *current* format actually shows: any field the
			// format doesn't include a token for (day, hour, minute, second, even year/month) is
			// reset to its default (day/month 1, year 1970, time 00:00:00) rather than left as-is.
			// Comparisons stay instant-based always (see the class comment) - this is the tool for
			// when you deliberately want two dates that merely *display* the same to also compare
			// equal, e.g. grouping transactions by month: format("MM.YYYY") then truncateToFormat()
			// before using the Date as a std::map/std::set key, so two September 2026 entries on
			// different days land in the same bucket instead of staying distinct keys.
			Date& truncateToFormat(void);
			Date new_truncateToFormat(void) const;

			// Parses against the current format (see the class comment on failure behavior).
			Date& operator=(const String& dateString);
			inline bool isValid(void) const { return m_valid; }

			bool operator==(const Date& other) const;
			inline bool operator!=(const Date& other) const { return !(*this == other); }
			bool operator<(const Date& other) const;
			inline bool operator<=(const Date& other) const { return !(other < *this); }
			inline bool operator>(const Date& other) const { return other < *this; }
			inline bool operator>=(const Date& other) const { return !(*this < other); }

			// Parses other against *this's current format for the comparison; false (and for
			// operator!= true) if it doesn't parse - see compare_with_string().
			bool operator==(const String& other) const;
			inline bool operator!=(const String& other) const { return !(*this == other); }
			bool operator<(const String& other) const;
			bool operator<=(const String& other) const;
			bool operator>(const String& other) const;
			bool operator>=(const String& other) const;

			// +=/-=/+/- with an integer are whole days; the Date-Date subtraction is the whole-day
			// difference between two moments (truncated, like integer division).
			inline Date& operator+=(i64 days) { m_epochSeconds += days * 86400LL; return *this; }
			inline Date& operator-=(i64 days) { m_epochSeconds -= days * 86400LL; return *this; }
			inline Date operator+(i64 days) const { Date copy = *this; copy += days; return copy; }
			inline Date operator-(i64 days) const { Date copy = *this; copy -= days; return copy; }
			inline i64 operator-(const Date& other) const { return (m_epochSeconds - other.m_epochSeconds) / 86400; }

			// Finer-grained adjustments than the whole-day operators above.
			inline Date& addDays(i64 days) { return (*this += days); }
			inline Date& addHours(i64 hours) { m_epochSeconds += hours * 3600LL; return *this; }
			inline Date& addMinutes(i64 minutes) { m_epochSeconds += minutes * 60LL; return *this; }
			inline Date& addSeconds(i64 seconds) { m_epochSeconds += seconds; return *this; }

			i32 getYear(void) const;
			u32 getMonth(void) const;
			u32 getDay(void) const;
			u32 getHour(void) const;
			u32 getMinute(void) const;
			u32 getSecond(void) const;
			// Raw seconds since 1970-01-01T00:00:00 - the canonical value comparisons use. Unlike
			// the getters above and toString(), this ignores the timezone offset below.
			inline i64 getEpochSeconds(void) const { return m_epochSeconds; }

			// A fixed UTC offset applied only when displaying (toString()/get()/the getters above),
			// not a real timezone (no DST, no named zones/database - this project has no tzdata
			// dependency to back that). Comparisons/arithmetic are unaffected, since they already
			// operate on the absolute moment in time regardless of how it's displayed.
			inline Date& setTimeZone(i32 utcOffsetMinutes) { m_utcOffsetMinutes = utcOffsetMinutes; return *this; }
			inline i32 getTimeZoneOffsetMinutes(void) const { return m_utcOffsetMinutes; }

		private:
			struct FormatPart
			{
				enum class eType : u8 { Literal, Year4, Year2, Month, Day, HourPadded, Hour, MinutePadded, Minute, SecondPadded, Second };
				eType  type { eType::Literal };
				String literal { "" };  // only meaningful when type == Literal
			};

			static i64 days_from_civil(i32 y, u32 m, u32 d);
			static void civil_from_days(i64 z, i32& outY, u32& outM, u32& outD);
			static bool is_leap_year(i32 y);
			static u32 days_in_month(i32 y, u32 m);

			// Used by range() to derive its step size from dest_fmt's finest field.
			enum class eGranularity : u8 { Second, Minute, Hour, Day, Month, Year };
			static i32 granularity_rank(FormatPart::eType t);
			static void truncate_to_granularity(eGranularity g, u32& mo, u32& d, u32& h, u32& mi, u32& s);

			void compile_format(const String& fmt);
			bool try_parse(const String& input);
			void decompose(i32& outY, u32& outMonth, u32& outDay, u32& outHour, u32& outMinute, u32& outSecond) const;
			i64 truncated_epoch_seconds(void) const;
			// Parses other against the current format; returns false ("not comparable") if it
			// doesn't parse, else true with outCmp set to -1/0/1 same as the usual three-way compare.
			bool compare_with_string(const String& other, i32& outCmp) const;

		private:
			i64 m_epochSeconds { 0 };
			i32 m_utcOffsetMinutes { 0 };
			bool m_valid { true };
			String m_format { "DD.MM.YYYY" };
			stdvec<FormatPart> m_compiledFormat;
	};
}
