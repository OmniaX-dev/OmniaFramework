#include "Time.hpp"
#include "Utils.hpp"
#include <thread>

namespace ostd
{
	StepTimer& StepTimer::create(double updatesPerSecond, StepTimer::Callback callback)
	{
	    m_targetDt = 1.0 / updatesPerSecond;
	    m_callback = std::move(callback);
	    m_prevTime = Clock::now();
	    m_accumulator = 0.0;
	    m_valid = true;
	    return *this;
	}

	void StepTimer::update(void)
	{
	    if (!m_valid) return;

		TimePoint currentTime = Clock::now();
		Duration frameDuration = currentTime - m_prevTime;
		m_prevTime = currentTime;

		// Convert to seconds
		double frameTime = std::chrono::duration<double>(frameDuration).count();

		// Clamp to prevent spiral of death (5 FPS minimum)
		constexpr double MAX_FRAME_TIME = 0.2;
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
	    }
	}



	// Utils
	void Utils::sleep(uint32_t __time, eTimeUnits __unit)
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
	uint64_t Utils::getRunningTime_ms(void)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count() - Utils::s_startTime_ms;
	}
}
