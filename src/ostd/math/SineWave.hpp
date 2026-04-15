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

#include <cmath>
#include <ostd/math/Geometry.hpp>

namespace ostd
{
	class SineWave
	{
		public:
			inline SineWave(void) { period = 1.0f; phase = 0.0f; amplitude = 1.0f; }
			inline SineWave(f32 pr, f32 am, f32 ph = 0.0f) { set(pr, am, ph); }
			inline SineWave& set(f32 pr, f32 am, f32 ph = 0.0f) { period = pr; phase = ph; amplitude = am; return *this; }
			inline f32 evaluate(f32 x) { return std::sin(phase + TWO_PI * x / period) * amplitude; }

		public:
			f32 period;
			f32 phase;
			f32 amplitude;
	};

	class AdditiveWave : public SineWave
	{
		public:
			inline AdditiveWave(void) { waves.clear(); }
			inline AdditiveWave& addWave(SineWave wave) { waves.push_back(wave); return *this; }
			inline AdditiveWave& addWave(AdditiveWave wave) { additiveWaves.push_back(wave); return *this; }
			f32 evaluate(f32 x);

		public:
			stdvec<SineWave> waves;
			stdvec<AdditiveWave> additiveWaves;
	};
}
