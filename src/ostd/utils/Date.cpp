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

#include "Date.hpp"
#include <ostd/io/Logger.hpp>
#include <ctime>

namespace ostd
{
	// days_from_civil / civil_from_days: the well-known proleptic-Gregorian day-count algorithm
	// by Howard Hinnant (public domain; this is the form that ships in many <chrono> standard
	// library implementations). Deliberately not mktime/timegm-based: those interpret a struct tm
	// against the process's local timezone (mktime) or aren't standard C++ (timegm), where this is
	// pure integer calendar math - same answer everywhere, no timezone involved at all.
	i64 Date::days_from_civil(i32 y, u32 m, u32 d)
	{
		y -= (m <= 2) ? 1 : 0;
		const i64 era = (y >= 0 ? y : y - 399) / 400;
		const u32 yoe = (u32)(y - era * 400);                               // [0, 399]
		const u32 doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;      // [0, 365]
		const u32 doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;               // [0, 146096]
		return era * 146097 + (i64)doe - 719468;
	}

	void Date::civil_from_days(i64 z, i32& outY, u32& outM, u32& outD)
	{
		z += 719468;
		const i64 era = (z >= 0 ? z : z - 146096) / 146097;
		const u32 doe = (u32)(z - era * 146097);                            // [0, 146096]
		const u32 yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
		const i32 y = (i32)yoe + (i32)(era * 400);
		const u32 doy = doe - (365 * yoe + yoe / 4 - yoe / 100);            // [0, 365]
		const u32 mp = (5 * doy + 2) / 153;                                 // [0, 11]
		outD = doy - (153 * mp + 2) / 5 + 1;                                // [1, 31]
		outM = mp + (mp < 10 ? 3 : -9);                                     // [1, 12]
		outY = y + (i32)(outM <= 2 ? 1 : 0);
	}

	bool Date::is_leap_year(i32 y)
	{
		return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
	}

	u32 Date::days_in_month(i32 y, u32 m)
	{
		static const u32 dim[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (m < 1 || m > 12)
			return 31;
		if (m == 2 && is_leap_year(y))
			return 29;
		return dim[m - 1];
	}

	Date::Date(void)
	{
		compile_format(m_format);
		m_epochSeconds = (i64)std::time(nullptr);
		m_valid = true;
	}

	Date::Date(i32 year, u32 month, u32 day, u32 hour, u32 minute, u32 second)
	{
		compile_format(m_format);
		m_epochSeconds = days_from_civil(year, month, day) * 86400LL + (i64)hour * 3600LL + (i64)minute * 60LL + (i64)second;
		m_valid = true;
	}

	void Date::compile_format(const String& fmt)
	{
		struct TokenDef { const char* text; u32 len; FormatPart::eType type; };
		// Order matters: longer tokens must be tried before shorter ones that are a prefix of
		// them, or e.g. "YYYY" would be mis-tokenized as two back-to-back "YY" parts.
		static const TokenDef tokens[] = {
			{ "YYYY", 4, FormatPart::eType::Year4 },
			{ "Min",  3, FormatPart::eType::MinutePadded },
			{ "min",  3, FormatPart::eType::Minute },
			{ "YY",   2, FormatPart::eType::Year2 },
			{ "MM",   2, FormatPart::eType::Month },
			{ "DD",   2, FormatPart::eType::Day },
			{ "HH",   2, FormatPart::eType::HourPadded },
			{ "hh",   2, FormatPart::eType::Hour },
			{ "SS",   2, FormatPart::eType::SecondPadded },
			{ "ss",   2, FormatPart::eType::Second },
		};

		m_format = fmt;
		m_compiledFormat.clear();
		const u32 len = fmt.len();
		String literalRun = "";

		auto flush_literal = [&](void) {
			if (!literalRun.empty())
			{
				m_compiledFormat.push_back({ FormatPart::eType::Literal, literalRun });
				literalRun = "";
			}
		};
		auto matches = [&](const char* tok, u32 tokLen, u32 pos) -> bool {
			if (pos + tokLen > len)
				return false;
			for (u32 k = 0; k < tokLen; k++)
			{
				if (fmt.at(pos + k) != tok[k])
					return false;
			}
			return true;
		};

		u32 i = 0;
		while (i < len)
		{
			bool matched = false;
			for (auto& t : tokens)
			{
				if (matches(t.text, t.len, i))
				{
					flush_literal();
					m_compiledFormat.push_back({ t.type, "" });
					i += t.len;
					matched = true;
					break;
				}
			}
			if (!matched)
			{
				literalRun.addChar(fmt.at(i));
				i++;
			}
		}
		flush_literal();
	}

	bool Date::try_parse(const String& input)
	{
		i32 year = 1970;
		u32 month = 1, day = 1, hour = 0, minute = 0, second = 0;

		u32 pos = 0;
		const u32 inLen = input.len();

		auto read_digits = [&](u32 minDigits, u32 maxDigits, u32& outValue) -> bool {
			const u32 start = pos;
			u32 count = 0;
			u32 value = 0;
			while (count < maxDigits && pos < inLen && input.at(pos) >= '0' && input.at(pos) <= '9')
			{
				value = value * 10 + (u32)(input.at(pos) - '0');
				pos++;
				count++;
			}
			if (count < minDigits)
			{
				pos = start;
				return false;
			}
			outValue = value;
			return true;
		};

		for (auto& part : m_compiledFormat)
		{
			u32 v = 0;
			switch (part.type)
			{
				case FormatPart::eType::Literal:
				{
					const u32 litLen = part.literal.len();
					if (pos + litLen > inLen)
						return false;
					for (u32 k = 0; k < litLen; k++)
					{
						if (input.at(pos + k) != part.literal.at(k))
							return false;
					}
					pos += litLen;
					break;
				}
				case FormatPart::eType::Year4:
					if (!read_digits(4, 4, v)) return false;
					year = (i32)v;
					break;
				case FormatPart::eType::Year2:
					if (!read_digits(2, 2, v)) return false;
					year = (i32)(v < 70 ? 2000 + v : 1900 + v);
					break;
				case FormatPart::eType::Month:
					if (!read_digits(2, 2, v) || v < 1 || v > 12) return false;
					month = v;
					break;
				case FormatPart::eType::Day:
					if (!read_digits(2, 2, v)) return false;
					day = v;  // range-checked against the actual month/year below
					break;
				case FormatPart::eType::HourPadded:
					if (!read_digits(2, 2, v) || v > 23) return false;
					hour = v;
					break;
				case FormatPart::eType::Hour:
					if (!read_digits(1, 2, v) || v > 23) return false;
					hour = v;
					break;
				case FormatPart::eType::MinutePadded:
					if (!read_digits(2, 2, v) || v > 59) return false;
					minute = v;
					break;
				case FormatPart::eType::Minute:
					if (!read_digits(1, 2, v) || v > 59) return false;
					minute = v;
					break;
				case FormatPart::eType::SecondPadded:
					if (!read_digits(2, 2, v) || v > 59) return false;
					second = v;
					break;
				case FormatPart::eType::Second:
					if (!read_digits(1, 2, v) || v > 59) return false;
					second = v;
					break;
			}
		}

		if (pos != inLen)
			return false;  // leftover characters the format didn't account for
		if (day < 1 || day > days_in_month(year, month))
			return false;

		m_epochSeconds = days_from_civil(year, month, day) * 86400LL + (i64)hour * 3600LL + (i64)minute * 60LL + (i64)second;
		m_valid = true;
		return true;
	}

	void Date::decompose(i32& outY, u32& outMonth, u32& outDay, u32& outHour, u32& outMinute, u32& outSecond) const
	{
		const i64 disp = m_epochSeconds + (i64)m_utcOffsetMinutes * 60LL;
		i64 days = disp / 86400;
		i64 secOfDay = disp % 86400;
		if (secOfDay < 0)
		{
			secOfDay += 86400;
			days -= 1;
		}
		civil_from_days(days, outY, outMonth, outDay);
		outHour = (u32)(secOfDay / 3600);
		outMinute = (u32)((secOfDay % 3600) / 60);
		outSecond = (u32)(secOfDay % 60);
	}

	String Date::toString(void) const
	{
		i32 y; u32 mo, d, h, mi, s;
		decompose(y, mo, d, h, mi, s);

		String out = "";
		for (auto& part : m_compiledFormat)
		{
			switch (part.type)
			{
				case FormatPart::eType::Literal:     out.add(part.literal); break;
				case FormatPart::eType::Year4:       out.add(String("").add((u32)y).addLeftPadding(4, '0')); break;
				case FormatPart::eType::Year2:       out.add(String("").add((u32)(((y % 100) + 100) % 100)).addLeftPadding(2, '0')); break;
				case FormatPart::eType::Month:       out.add(String("").add(mo).addLeftPadding(2, '0')); break;
				case FormatPart::eType::Day:         out.add(String("").add(d).addLeftPadding(2, '0')); break;
				case FormatPart::eType::HourPadded:  out.add(String("").add(h).addLeftPadding(2, '0')); break;
				case FormatPart::eType::Hour:        out.add(String("").add(h)); break;
				case FormatPart::eType::MinutePadded:out.add(String("").add(mi).addLeftPadding(2, '0')); break;
				case FormatPart::eType::Minute:      out.add(String("").add(mi)); break;
				case FormatPart::eType::SecondPadded:out.add(String("").add(s).addLeftPadding(2, '0')); break;
				case FormatPart::eType::Second:      out.add(String("").add(s)); break;
			}
		}
		return out;
	}

	Date& Date::format(const String& fmt)
	{
		compile_format(fmt);
		return *this;
	}

	Date Date::new_format(const String& fmt) const
	{
		Date copy = *this;
		copy.compile_format(fmt);
		return copy;
	}

	Date& Date::operator=(const String& dateString)
	{
		if (!try_parse(dateString))
		{
			OX_WARN("ostd::Date: failed to parse \"%s\" against format \"%s\".", dateString.c_str(), m_format.c_str());
			m_valid = false;
		}
		return *this;
	}

	bool Date::operator==(const Date& other) const { return m_epochSeconds == other.m_epochSeconds; }
	bool Date::operator<(const Date& other) const { return m_epochSeconds < other.m_epochSeconds; }

	bool Date::compare_with_string(const String& other, i32& outCmp) const
	{
		Date tmp;
		tmp.format(m_format);
		if (!tmp.try_parse(other))
			return false;
		outCmp = (m_epochSeconds < tmp.m_epochSeconds) ? -1 : (m_epochSeconds > tmp.m_epochSeconds ? 1 : 0);
		return true;
	}

	bool Date::operator==(const String& other) const { i32 c; return compare_with_string(other, c) && c == 0; }
	bool Date::operator<(const String& other) const { i32 c; return compare_with_string(other, c) && c < 0; }
	bool Date::operator<=(const String& other) const { i32 c; return compare_with_string(other, c) && c <= 0; }
	bool Date::operator>(const String& other) const { i32 c; return compare_with_string(other, c) && c > 0; }
	bool Date::operator>=(const String& other) const { i32 c; return compare_with_string(other, c) && c >= 0; }

	i32 Date::getYear(void) const { i32 y; u32 mo, d, h, mi, s; decompose(y, mo, d, h, mi, s); return y; }
	u32 Date::getMonth(void) const { i32 y; u32 mo, d, h, mi, s; decompose(y, mo, d, h, mi, s); return mo; }
	u32 Date::getDay(void) const { i32 y; u32 mo, d, h, mi, s; decompose(y, mo, d, h, mi, s); return d; }
	u32 Date::getHour(void) const { i32 y; u32 mo, d, h, mi, s; decompose(y, mo, d, h, mi, s); return h; }
	u32 Date::getMinute(void) const { i32 y; u32 mo, d, h, mi, s; decompose(y, mo, d, h, mi, s); return mi; }
	u32 Date::getSecond(void) const { i32 y; u32 mo, d, h, mi, s; decompose(y, mo, d, h, mi, s); return s; }
}
