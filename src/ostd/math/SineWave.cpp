#include "SineWave.hpp"

namespace ostd
{
	f32 AdditiveWave::evaluate(f32 x)
	{
		f32 y = 0.0f;
		for (auto& wave : waves)
			y += wave.evaluate(x);
		for (auto& wave : additiveWaves)
			y += wave.evaluate(x);
		return y;
	}
} 
