/*
    OmniaFramework - A collection of useful functionality
    Copyright (C) 2025  OmniaX-Dev

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

#include <ostd/data/Types.hpp>
#include <ostd/string/String.hpp>
#include <functional>

//Game Time constants
#define TM_R_SECONDS_FOR_G_MINUTE 1
#define TM_G_MINUTES_FOR_G_HOUR 60
#define TM_G_HOURS_FOR_G_DAY 24
#define TM_G_DAYS_FOR_G_LONG_MONTH 31
#define TM_G_DAYS_FOR_G_MEDIUM_MONTH 30
#define TM_G_DAYS_FOR_G_SHORT_MONTH 28

namespace ostd
{
	enum class eTimeUnits
	{
		Seconds = 0,
		Milliseconds,
		Microseconds,
		Nanoseconds
	};
	enum class eMonths
	{
		January = 0,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};


	template <class T>
	class _Counter
	{
		public:
			inline _Counter(void) { m_count = 200; m_current = 0; }
			inline _Counter(T count, T step = 1) { m_count = count; m_current = 0; m_step = step; }
			inline void setCount(T count) { m_count = count; m_counting = false; m_current = 0; }
			inline T getCount(void) { return m_count; }
			inline T getCurrent(void) { return m_current; };
			inline T getStep(void) { return m_step; }
			inline void start(void) { m_current = 0; m_counting = true; }
			inline void stop(void) { m_current = 0; m_counting = false; }
			inline bool isDone(void) { return !m_counting; }
			inline bool isCounting(void) { return m_counting; }
			inline void update(void)
			{
				if (!m_counting) return;
				m_current += m_step;
				if (m_current > m_count)
					stop();
			}

		private:
			T m_current { 0 };
			T m_count { 200 };
			T m_step { 1 };
			bool m_counting { false };
	};

	typedef _Counter<u64> Counter;
	typedef _Counter<f32> FloatCounter;
	typedef _Counter<f64> DoubleCounter;

	class OutputHandlerBase;
	class Timer
	{
		public:
			inline Timer(void) { m_started = false; m_current = 0; m_timeUnit = eTimeUnits::Nanoseconds; m_dest = nullptr; }
			u64 start(bool print = true, const String& name = "", eTimeUnits timeUnit = eTimeUnits::Nanoseconds, OutputHandlerBase* __destination = nullptr);
			u64 startCount(eTimeUnits timeUnit = eTimeUnits::Nanoseconds);
			u64 end(bool print = true);
			u64 endCount(bool stop = true);
			u64 restart(eTimeUnits timeUnit = eTimeUnits::Nanoseconds);

			static u64 getEpoch(eTimeUnits timeUnit = eTimeUnits::Milliseconds);

			inline const String& getName(void) const { return m_name; }
			inline u64 read(void) { if (!m_started) return 0; return endCount(false); }

		private:
			bool m_started;
			i64 m_current;
			eTimeUnits m_timeUnit;
			String m_name;
			OutputHandlerBase* m_dest;
	};

	class StepTimer
	{
		public:
		    using Callback = std::function<void(f64 dt)>;
			using Clock = std::chrono::high_resolution_clock;
		    using TimePoint = Clock::time_point;
		    using Duration = Clock::duration;

		public:
		    StepTimer() = default;
		    inline StepTimer(f64 updatesPerSecond, Callback callback) { create(updatesPerSecond, callback); }
		    StepTimer& create(f64 updatesPerSecond, Callback callback);
		    void update(void);
		    void reset(void);
			inline f64 getInterpolationAlpha(void) const { return m_valid && m_targetDt > 0.0 ? m_accumulator / m_targetDt : 0.0; }
		    inline void invalidate(void) { m_valid = false; }
		    inline bool isValid(void) const { return m_valid; }

		private:
			TimePoint m_prevTime;
		    f64 m_targetDt { 0.0 };
		    Callback m_callback { nullptr };
		    f64 m_accumulator { 0.0 };
		    bool m_valid { false };
	};

	struct GameClock
	{
		public:
			GameClock(void);
			const f32& start(void);
			inline const f32& getTimeOfDay(void) const { return m_timeOfDay; }
			String asString(void);
			void update(void);

		private:
			String getFormattedTime(void);
			String convertMonth(void);

		public:
			u8 minutes;
			u8 hours;
			u16 days;
			u8 months;
			u16 years;

		private:
			Timer m_rtClock;
			f32 m_timeOfDay;
			f32 m_totalSeconds;
	};

	class LocalTime
	{
		public:
			String getFullString(bool include_date = true, bool include_time = true, bool day_name = true, bool month_as_name = true, bool include_seconds = true) const;
			inline String getDateString(bool day_name = true, bool month_as_name = true) { return getFullString(true, false, day_name, month_as_name, false); }
			inline String getTimeString(bool include_seconds = true) { return getFullString(false, true, false, false, include_seconds); }
			i32 hours(void) const;
			i32 minutes(void) const;
			i32 seconds(void) const;
			i32 day(void) const;
			i32 month(void) const;
			i32 year(void) const;
			i32 weekDay(void) const;
			String shours(bool leading_zero = true) const;
			String sminutes(bool leading_zero = true) const;
			String sseconds(bool leading_zero = true) const;
			String sday(bool leading_zero = true) const;
			String smonth(bool leading_zero = true, bool month_name = true) const;
			String syear(void) const;
			String sWeekDay(bool day_name = true) const;

		protected:
			virtual String monthToText(i32 month) const;
			virtual String weekDayToText(i32 day) const;
	};
	typedef LocalTime LocalTime_EN;
	class LocalTime_IT : public LocalTime
	{
		protected:
			String monthToText(i32 month) const override;
			String weekDayToText(i32 day) const override;
	};
	class LocalTime_ES : public LocalTime
	{
		protected:
			String monthToText(i32 month) const override;
			String weekDayToText(i32 day) const override;
	};
	class LocalTime_DE : public LocalTime
	{
		protected:
			String monthToText(i32 month) const override;
			String weekDayToText(i32 day) const override;
	};

	class Time
	{
		public:
			static void startRunningTimer(void);
			static inline const u64 getStartTime(void) { return s_startTime_ms; }
			static void sleep(u32 __time, eTimeUnits __unit = eTimeUnits::Milliseconds);
			static u64 getRunningTime_ms(void);
			static String secondsToFormattedString(i32 totalSeconds);

		private:
			inline static u64 s_startTime_ms;

	};
}
