#include "Time.hpp"
#include <thread>
#include "../vendor/TermColor.hpp"
#include "../io/IOHandlers.hpp"

#define __get_local_time() \
		std::time_t __cur_t = std::time(0); \
		std::tm* __now_t = std::localtime(&__cur_t);

namespace ostd
{
	GameClock::GameClock(void)
	{
		minutes = 0;
		hours = 0;
		days = 0;
		months = 0;
		years = 2022;
		m_timeOfDay = 0.0f;
		m_totalSeconds = 0.0f;
	}

	const f32& GameClock::start(void)
	{
		m_rtClock.start(false, "", eTimeUnits::Seconds);
		m_timeOfDay = CAP((1.0f / ((f32)(TM_G_MINUTES_FOR_G_HOUR * TM_G_HOURS_FOR_G_DAY))) * ((hours * TM_G_MINUTES_FOR_G_HOUR) + (minutes)), 1.0f);
		return m_timeOfDay;
	}

	String GameClock::asString(void)
	{
		std::ostringstream ss;
		ss << "Time: " << getFormattedTime() << " / ";
		ss << (i32)(days + 1) << " " << convertMonth() << " " << (i32)(years);
		return String(ss.str());
	}

	void GameClock::update(void)
	{
		i64 elapsed = m_rtClock.start(false, "", eTimeUnits::Seconds);
		if (hours == 255)
			hours = TM_G_HOURS_FOR_G_DAY - 1;
		else if (hours >= TM_G_HOURS_FOR_G_DAY)
			hours = 0;
		if (elapsed >= TM_R_SECONDS_FOR_G_MINUTE)
		{
			minutes++;
			if (minutes >= TM_G_MINUTES_FOR_G_HOUR)
			{
				hours++;
				if (hours >= TM_G_HOURS_FOR_G_DAY)
				{
					days++;
					if ((months == (u8)eMonths::January || months == (u8)eMonths::March ||
						months == (u8)eMonths::May || months == (u8)eMonths::July ||
						months == (u8)eMonths::August || months == (u8)eMonths::October ||
						months == (u8)eMonths::December) && days >= TM_G_DAYS_FOR_G_LONG_MONTH)
					{
						months++;
						if (months > (u8)eMonths::December)
						{
							years++;
							months = (u8)eMonths::January;
						}
						days = 0;
					}
					else if ((months == (u8)eMonths::April || months == (u8)eMonths::June ||
							months == (u8)eMonths::September || months == (u8)eMonths::November) && days >= TM_G_DAYS_FOR_G_MEDIUM_MONTH)
					{
						months++;
						days = 0;
					}
					else if (months == (u8)eMonths::February)
					{
						if ((years % 4 == 0 && days >= TM_G_DAYS_FOR_G_SHORT_MONTH + 1) ||
							(years % 4 != 0 && days >= TM_G_DAYS_FOR_G_SHORT_MONTH))
						{
							months++;
							days = 0;
						}
					}
					hours = 0;
				}
				minutes = 0;
			}
			m_totalSeconds += elapsed;
			m_rtClock.start(false, "", eTimeUnits::Seconds);
			m_timeOfDay = CAP((1.0f / ((f32)(TM_G_MINUTES_FOR_G_HOUR * TM_G_HOURS_FOR_G_DAY))) * ((hours * TM_G_MINUTES_FOR_G_HOUR) + (minutes)), 1.0f);
		}
	}

	String GameClock::getFormattedTime(void)
	{
		bool zh = (i32)(hours / 10) < 1;
		bool zm = (i32)(minutes / 10) < 1;
		std::ostringstream ss;
		ss << (zh ? "0" : "") << (i32)hours << ":" << (zm ? "0" : "") << (i32)minutes;
		return String(ss.str());
	}

	String GameClock::convertMonth(void)
	{
		switch (months)
		{
			case (u8)eMonths::January:
				return "January";
			case (u8)eMonths::February:
				return "February";
			case (u8)eMonths::March:
				return "March";
			case (u8)eMonths::April:
				return "April";
			case (u8)eMonths::May:
				return "May";
			case (u8)eMonths::June:
				return "June";
			case (u8)eMonths::July:
				return "July";
			case (u8)eMonths::August:
				return "August";
			case (u8)eMonths::September:
				return "September";
			case (u8)eMonths::October:
				return "October";
			case (u8)eMonths::November:
				return "November";
			case (u8)eMonths::December:
				return "December";
			default:
				break;
		}
		return "_MONTH_";
	}



	String LocalTime::getFullString(bool include_date, bool include_time, bool day_name, bool month_as_name, bool include_seconds) const
	{
		std::ostringstream ss;
		if (include_date)
		{
			if (day_name) ss << sWeekDay(true) << " ";
			ss << sday(true);
			if (month_as_name) ss << " " << smonth(false, true) << " ";
			else ss << "." << smonth(true, false) << ".";
			ss << syear();
		}
		if (include_time)
		{
			if (include_date) ss << " - ";
			ss << shours(true) << ":" << sminutes(true);
			if (include_seconds) ss << ":" << sseconds(true);
		}
		return ss.str();
	}

	i32 LocalTime::hours(void) const
	{
		__get_local_time();
		return __now_t->tm_hour;
	}

	i32 LocalTime::minutes(void) const
	{
		__get_local_time();
		return __now_t->tm_min;
	}

	i32 LocalTime::seconds(void) const
	{
		__get_local_time();
		return __now_t->tm_sec;
	}

	i32 LocalTime::day(void) const
	{
		__get_local_time();
		return __now_t->tm_mday;
	}

	i32 LocalTime::month(void) const
	{
		__get_local_time();
		return __now_t->tm_mon + 1;
	}

	i32 LocalTime::year(void) const
	{
		__get_local_time();
		return __now_t->tm_year + 1900;
	}

	i32 LocalTime::weekDay(void) const
	{
		__get_local_time();
		return __now_t->tm_wday;
	}

	String LocalTime::shours(bool leading_zero) const
	{
		std::ostringstream ss;
		i32 h = hours();
		if (leading_zero && h < 10)
			ss << "0" << (i32)h;
		else
			ss << (i32)h;
		return ss.str();
	}

	String LocalTime::sminutes(bool leading_zero) const
	{
		std::ostringstream ss;
		i32 h = minutes();
		if (leading_zero && h < 10)
			ss << "0" << (i32)h;
		else
			ss << (i32)h;
		return ss.str();
	}

	String LocalTime::sseconds(bool leading_zero) const
	{
		std::ostringstream ss;
		i32 h = seconds();
		if (leading_zero && h < 10)
			ss << "0" << (i32)h;
		else
			ss << (i32)h;
		return ss.str();
	}

	String LocalTime::sday(bool leading_zero) const
	{
		std::ostringstream ss;
		i32 h = day();
		if (leading_zero && h < 10)
			ss << "0" << (i32)h;
		else
			ss << (i32)h;
		return ss.str();
	}

	String LocalTime::smonth(bool leading_zero, bool month_name) const
	{
		i32 h = month();
		if (month_name) return monthToText(h);
		std::ostringstream ss;
		if (leading_zero && h < 10)
			ss << "0" << (i32)h;
		else
			ss << (i32)h;
		return ss.str();
	}

	String LocalTime::syear(void) const
	{
		std::ostringstream ss;
		i32 h = year();
		ss << (i32)h;
		return ss.str();
	}

	String LocalTime::sWeekDay(bool day_name) const
	{
		i32 h = weekDay();
		if (day_name)
			return weekDayToText(h);
		std::ostringstream ss;
		ss << (i32)h;
		return ss.str();
	}

	String LocalTime::monthToText(i32 month) const
	{
		switch (month)
		{
			case 1: return "January";
			case 2: return "February";
			case 3: return "March";
			case 4: return "April";
			case 5: return "May";
			case 6: return "June";
			case 7: return "July";
			case 8: return "August";
			case 9: return "September";
			case 10: return "October";
			case 11: return "November";
			case 12: return "December";
			default: return "Unknown Month";
		}
	}

	String LocalTime::weekDayToText(i32 day) const
	{
		switch (day)
		{
			case 0: return "Sun";
			case 1: return "Mon";
			case 2: return "Tue";
			case 3: return "Wed";
			case 4: return "Thu";
			case 5: return "Fri";
			case 6: return "Sat";
			default: return "Unknown Day";
		}
	}

	String LocalTime_IT::monthToText(i32 month) const
	{
		switch (month)
		{
			case 1: return "Gennaio";
			case 2: return "Febraio";
			case 3: return "Marzo";
			case 4: return "Aprile";
			case 5: return "Maggio";
			case 6: return "Giugno";
			case 7: return "Luglio";
			case 8: return "Agosto";
			case 9: return "Settembre";
			case 10: return "Ottobre";
			case 11: return "Novembre";
			case 12: return "Dicembre";
			default: return "Mese sconosciuto";
		}
	}

	String LocalTime_IT::weekDayToText(i32 day) const
	{
		switch (day)
		{
			case 0: return "Dom";
			case 1: return "Lun";
			case 2: return "Mar";
			case 3: return "Mer";
			case 4: return "Gio";
			case 5: return "Ven";
			case 6: return "Sab";
			default: return "Giorno Sconosciuto";
		}
	}

	String LocalTime_ES::monthToText(i32 month) const
	{
		switch (month)
		{
			case 1: return "Enero";
			case 2: return "Febrero";
			case 3: return "Marzo";
			case 4: return "Abril";
			case 5: return "Mayo";
			case 6: return "Junio";
			case 7: return "Julio";
			case 8: return "Agosto";
			case 9: return "Septiembre";
			case 10: return "Octubre";
			case 11: return "Noviembre";
			case 12: return "Diciembre";
			default: return "Mes desconocido";
		}
	}

	String LocalTime_ES::weekDayToText(i32 day) const
	{
		switch (day)
		{
			case 0: return "Domingo";
			case 1: return "Lunes";
			case 2: return "Martes";
			case 3: return "Miercoles";
			case 4: return "Jueves";
			case 5: return "Viernes";
			case 6: return "Sabado";
			default: return "Dia desconoscido";
		}
	}

	String LocalTime_DE::monthToText(i32 month) const
	{
		switch (month)
		{
			case 1: return "Januar";
			case 2: return "Februar";
			case 3: return "Marz";
			case 4: return "April";
			case 5: return "May";
			case 6: return "Juni";
			case 7: return "July";
			case 8: return "August";
			case 9: return "September";
			case 10: return "Oktuber";
			case 11: return "November";
			case 12: return "Dizember";
			default: return "Unknown day";
		}
	}

	String LocalTime_DE::weekDayToText(i32 day) const
	{
		switch (day)
		{
			case 0: return "So";
			case 1: return "Mo";
			case 2: return "Di";
			case 3: return "Mi";
			case 4: return "Do";
			case 5: return "Fr";
			case 6: return "Sa";
			default: return "Unknown day";
		}
	}



	u64 Timer::start(bool print, const String& name, eTimeUnits timeUnit, OutputHandlerBase* __destination)
	{
		m_timeUnit = timeUnit;
		m_started = true;
		m_name = name;
		if (print)
		{
			if (__destination == nullptr)
			{
				std::cout << "\n" << termcolor::magenta << "====>   ";
				std::cout << termcolor::cyan << "Starting test for [";
				std::cout << termcolor::green << m_name;
				std::cout << termcolor::cyan << "]";
				std::cout << termcolor::magenta << "   <====";
				std::cout << termcolor::reset << "\n";
			}
			else
			{
				m_dest = __destination;
				m_dest->nl().fg("magenta").p("====>   ");
				m_dest->fg("cyan").p("Starting test for [");
				m_dest->fg("green").p(m_name);
				m_dest->fg("cyan").p("]");
				m_dest->fg("magenta").p("   <====");
				m_dest->reset().nl();
			}
		}
		switch (m_timeUnit)
		{
			case eTimeUnits::Nanoseconds:
				m_current = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			case eTimeUnits::Microseconds:
				m_current = std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			case eTimeUnits::Milliseconds:
				m_current = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			case eTimeUnits::Seconds:
				m_current = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			default: m_started = false; return 0;
		}
		m_started = false;
		return 0;
	}

	u64 Timer::startCount(eTimeUnits timeUnit)
	{
		m_timeUnit = timeUnit;
		m_started = true;
		switch (m_timeUnit)
		{
			case eTimeUnits::Nanoseconds:
				m_current = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			case eTimeUnits::Microseconds:
				m_current = std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			case eTimeUnits::Milliseconds:
				m_current = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			case eTimeUnits::Seconds:
				m_current = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				return m_current;
			default: m_started = false; return 0;
		}
		m_started = false;
		return 0;
	}

	u64 Timer::end(bool print)
	{
		if (!m_started) return 0;
		m_started = false;
		m_dest = nullptr;
		i64 diff;
		String unit;
		switch (m_timeUnit)
		{
			case eTimeUnits::Nanoseconds:
				diff = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				unit = " ns";
			break;
			case eTimeUnits::Microseconds:
				diff = std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				unit = " us";
			break;
			case eTimeUnits::Milliseconds:
				diff = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				unit = " ms";
			break;
			case eTimeUnits::Seconds:
				diff = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now().time_since_epoch()).count();
				unit = " s";
			break;
			default: return 0;
		}
		diff -= m_current;
		if (print)
		{
			if (m_dest == nullptr)
			{
				std::cout << termcolor::magenta << "====>   ";
				std::cout << termcolor::cyan << "Test for [";
				std::cout << termcolor::green << m_name;
				std::cout << termcolor::cyan << "] took ";
				std::cout << termcolor::bright_blue << diff << unit;
				std::cout << termcolor::magenta << "   <====";
				std::cout << termcolor::reset << "\n";
			}
			else
			{
				m_dest->fg("magenta").p("====>   ");
				m_dest->fg("cyan").p("Test for [");
				m_dest->fg("green").p(m_name);
				m_dest->fg("cyan").p("] took ");
				m_dest->fg("b-blue").p(diff).p(unit);
				m_dest->nl().nl().fg("magenta").p("   <====");
				m_dest->reset().nl();
			}
		}
		return diff;
	}

	u64 Timer::endCount(bool stop)
	{
		if (!m_started) return 0;
		m_started = !stop;
		i64 diff;
		switch (m_timeUnit)
		{
			case eTimeUnits::Nanoseconds:
				diff = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			case eTimeUnits::Microseconds:
				diff = std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			case eTimeUnits::Milliseconds:
				diff = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			case eTimeUnits::Seconds:
				diff = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			default: return 0;
		}
		diff -= m_current;
		return diff;
	}

	u64 Timer::restart(eTimeUnits timeUnit)
	{
		if (!m_started)
		{
			startCount(timeUnit);
			return 0;
		}
		u64 elapsed = endCount();
		startCount(timeUnit);
		return elapsed;
	}

	u64 Timer::getEpoch(eTimeUnits timeUnit)
	{
		switch (timeUnit)
		{
			case eTimeUnits::Nanoseconds:
				return std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			case eTimeUnits::Microseconds:
				return std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			case eTimeUnits::Milliseconds:
				return std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			case eTimeUnits::Seconds:
				return std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now().time_since_epoch()).count();
			break;
			default: return 0;
		}
		return 0;
	}



	StepTimer& StepTimer::create(f64 updatesPerSecond, StepTimer::Callback callback, StopConditionCallback stopCondition, bool stopped)
	{
		m_targetDt = 1.0 / updatesPerSecond;
		m_callback = std::move(callback);
		m_stopCondition = std::move(stopCondition);
		m_prevTime = Clock::now();
		m_accumulator = 0.0;
		m_stopped = stopped;
		m_valid = true;
		return *this;
	}

	void StepTimer::update(void)
	{
		if (!m_valid || m_stopped || !m_callback) return;
		if (m_stopCondition && m_stopCondition())
		{
			m_stopped = true;
			return;
		}

		TimePoint currentTime = Clock::now();
		Duration frameDuration = currentTime - m_prevTime;
		m_prevTime = currentTime;

		// Convert to seconds
		f64 frameTime = std::chrono::duration<f64>(frameDuration).count();

		// Clamp to prevent spiral of death (5 FPS minimum)
		constexpr f64 MAX_FRAME_TIME = 0.2;
		if (frameTime > MAX_FRAME_TIME)
			frameTime = MAX_FRAME_TIME;

		m_accumulator += frameTime;

		while (m_accumulator >= m_targetDt)
		{
			m_callback(m_targetDt);
			m_accumulator -= m_targetDt;
		}
	}

	void StepTimer::reset(void)
	{
		if (m_valid)
		{
			m_accumulator = 0.0;
			m_prevTime = Clock::now();
			m_stopped = false;
		}
	}




	void Time::startRunningTimer(void)
	{
		s_startTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	void Time::sleep(u32 __time, eTimeUnits __unit)
	{
		switch (__unit)
		{
		case eTimeUnits::Seconds:
			std::this_thread::sleep_for(std::chrono::seconds(__time));
			break;
		case eTimeUnits::Milliseconds:
			std::this_thread::sleep_for(std::chrono::milliseconds(__time));
			break;
		case eTimeUnits::Microseconds:
			std::this_thread::sleep_for(std::chrono::microseconds(__time));
			break;
		case eTimeUnits::Nanoseconds:
			std::this_thread::sleep_for(std::chrono::nanoseconds(__time));
			break;
		default: break;
		}
	}

	u64 Time::getRunningTime_ms(void)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count() - s_startTime_ms;
	}

	String Time::secondsToFormattedString(i32 totalSeconds)
	{
			i32 hours   = totalSeconds / 3600;
			i32 minutes = (totalSeconds % 3600) / 60;
			i32 seconds = totalSeconds % 60;
			String fmtstr = "";
			fmtstr.add(String("").add(hours).addLeftPadding(2, '0')).add(":");
			fmtstr.add(String("").add(minutes).addLeftPadding(2, '0')).add(":");
			fmtstr.add(String("").add(seconds).addLeftPadding(2, '0'));
			return fmtstr;
	}
}
