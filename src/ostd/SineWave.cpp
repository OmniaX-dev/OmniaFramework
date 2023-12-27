#include "SineWave.hpp"

namespace ostd
{
	float AdditiveWave::evaluate(float x)
	{
		float y = 0.0f;
		for (auto& wave : waves)
			y += wave.evaluate(x);
		for (auto& wave : additiveWaves)
			y += wave.evaluate(x);
		return y;
	}
} 
