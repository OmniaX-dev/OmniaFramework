#ifndef __SINE_WAVE_HPP__
#define __SINE_WAVE_HPP__

#include <vector>
#include <cmath>

#include <ostd/Defines.hpp>

namespace ostd
{
	class SineWave
	{
		public:
			inline SineWave(void) { period = 1.0f; phase = 0.0f; amplitude = 1.0f; }
			inline SineWave(float pr, float am, float ph = 0.0f) { set(pr, am, ph); }
			inline SineWave& set(float pr, float am, float ph = 0.0f) { period = pr; phase = ph; amplitude = am; return *this; }
			inline float evaluate(float x) { return std::sin(phase + TWO_PI * x / period) * amplitude; }

		public:
			float period;
			float phase;
			float amplitude;
	};

	class AdditiveWave : public SineWave
	{
		public:
			inline AdditiveWave(void) { waves.clear(); }
			inline AdditiveWave& addWave(SineWave wave) { waves.push_back(wave); return *this; }
			inline AdditiveWave& addWave(AdditiveWave wave) { additiveWaves.push_back(wave); return *this; }
			float evaluate(float x);

		public:
			std::vector<SineWave> waves;
			std::vector<AdditiveWave> additiveWaves;
	};
} 

#endif
