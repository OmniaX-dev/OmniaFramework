#pragma once

#include <ostd/Types.hpp>
#include <ostd/String.hpp>
#include <functional>

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

	typedef _Counter<uint64_t> Counter;
	typedef _Counter<float> FloatCounter;
	typedef _Counter<double> DoubleCounter;

	class OutputHandlerBase;
	class Timer
	{
		public:
			inline Timer(void) { m_started = false; m_current = 0; m_timeUnit = eTimeUnits::Nanoseconds; m_dest = nullptr; }
			uint64_t start(bool print = true, String name = "", eTimeUnits timeUnit = eTimeUnits::Nanoseconds, OutputHandlerBase* __destination = nullptr);
			uint64_t startCount(eTimeUnits timeUnit = eTimeUnits::Nanoseconds);
			uint64_t end(bool print = true);
			uint64_t endCount(bool stop = true);

			static uint64_t getEpoch(eTimeUnits timeUnit = eTimeUnits::Milliseconds);

			inline const String& getName(void) const { return m_name; }
			inline uint64_t read(void) { if (!m_started) return 0; return endCount(false); }

		private:
			bool m_started;
			int64_t m_current;
			eTimeUnits m_timeUnit;
			String m_name;
			OutputHandlerBase* m_dest;
	};

	class StepTimer
	{
		public:
		    using Callback = std::function<void(double dt)>;
			using Clock = std::chrono::high_resolution_clock;
		    using TimePoint = Clock::time_point;
		    using Duration = Clock::duration;

		public:
		    StepTimer() = default;
		    inline StepTimer(double updatesPerSecond, Callback callback) { create(updatesPerSecond, callback); }
		    StepTimer& create(double updatesPerSecond, Callback callback);
		    void update(void);
		    void reset(void);
			inline double getInterpolationAlpha(void) const { return m_valid && m_targetDt > 0.0 ? m_accumulator / m_targetDt : 0.0; }
		    inline void invalidate(void) { m_valid = false; }
		    inline bool isValid(void) const { return m_valid; }

		private:
			TimePoint m_prevTime;
		    double m_targetDt { 0.0 };
		    Callback m_callback { nullptr };
		    double m_accumulator { 0.0 };
		    bool m_valid { false };
	};

	struct GameClock
	{
		public:
			GameClock(void);
			const float& start(void);
			inline const float& getTimeOfDay(void) const { return m_timeOfDay; }
			String asString(void);
			void update(void);

		private:
			String getFormattedTime(void);
			String convertMonth(void);

		public:
			uint8_t minutes;
			uint8_t hours;
			uint16_t days;
			uint8_t months;
			uint16_t years;

		private:
			Timer m_rtClock;
			float m_timeOfDay;
			float m_totalSeconds;
	};

	class LocalTime
	{
		public:
			String getFullString(bool include_date = true, bool include_time = true, bool day_name = true, bool month_as_name = true, bool include_seconds = true) const;
			inline String getDateString(bool day_name = true, bool month_as_name = true) { return getFullString(true, false, day_name, month_as_name, false); }
			inline String getTimeString(bool include_seconds = true) { return getFullString(false, true, false, false, include_seconds); }
			int32_t hours(void) const;
			int32_t minutes(void) const;
			int32_t seconds(void) const;
			int32_t day(void) const;
			int32_t month(void) const;
			int32_t year(void) const;
			int32_t weekDay(void) const;
			String shours(bool leading_zero = true) const;
			String sminutes(bool leading_zero = true) const;
			String sseconds(bool leading_zero = true) const;
			String sday(bool leading_zero = true) const;
			String smonth(bool leading_zero = true, bool month_name = true) const;
			String syear(void) const;
			String sWeekDay(bool day_name = true) const;

		protected:
			virtual String monthToText(int32_t month) const;
			virtual String weekDayToText(int32_t day) const;
	};
	typedef LocalTime LocalTime_EN;
	class LocalTime_IT : public LocalTime
	{
		protected:
			String monthToText(int32_t month) const override;
			String weekDayToText(int32_t day) const override;
	};
	class LocalTime_ES : public LocalTime
	{
		protected:
			String monthToText(int32_t month) const override;
			String weekDayToText(int32_t day) const override;
	};
	class LocalTime_DE : public LocalTime
	{
		protected:
			String monthToText(int32_t month) const override;
			String weekDayToText(int32_t day) const override;
	};
}
