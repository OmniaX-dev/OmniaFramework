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

#include <random>
#include <climits>
#include <ostd/math/Geometry.hpp>
#include <ostd/vendor/FastNoiseLite.hpp>

namespace ostd
{
	class RandomGenerator
	{
		public:
			inline RandomGenerator(bool auto_seed = true) { if (auto_seed) autoSeed(); }
			inline RandomGenerator(i64 _seed) { seed(_seed); }

			void seed(i64 _seed);
			void autoSeed(void);
			i64 getSeed(void);

			f32 getf32(f32 min = 0.0f, f32 max = 1.0f);
			f64 getf64(f64 min = 0.0f, f64 max = 1.0f);
			i64 geti64(i64 min = LLONG_MIN, i64 max = LLONG_MAX);
			i32 geti32(i32 min = INT_MIN, i32 max = INT_MAX);
			i16 geti16(i16 min = SHRT_MIN, i16 max = SHRT_MAX);
			i8 geti8(i8 min = CHAR_MIN, i8 max = CHAR_MAX);
			u64 getui64(u64 min = 0, u64 max = ULLONG_MAX);
			u32 getui32(u32 min = 0, u32 max = UINT_MAX);
			u16 getui16(u16 min = 0, u16 max = USHRT_MAX);
			u8 getui8(u8 min = 0, u8 max = UCHAR_MAX);
			bool getb(f32 true_percentage = 0.5f);
			Vec2 getVec2(f32 min = 0.0f, f32 max = 1.0f, bool match_xy = false);
			Vec2 getVec2(Vec2 minmax_x = { 0.0f, 1.0f }, Vec2 minmax_y = { 0.0f, 1.0f });
			f32 getOpenSimplex2D(f32 x, f32 y);

			inline FastNoiseLite& getNoiseGenerator(void) { return m_noiseGen; }

			template <typename T>
			inline T& getFromStdVec(stdvec<T>& list)
			{
				u64 index = cast<u64>(geti64(0, list.size()));
				return list[index];
			}

		private:
			template <typename T>
			inline void __swap_if_first_is_bigger(T& n1, T& n2)
			{
				if (n1 <= n2) return;
				T n3 = n1;
				n1 = n2;
				n2 = n3;
			}

		private:
			i64 m_seed { 0 };
			std::mt19937_64 m_engine;
			FastNoiseLite m_noiseGen;
	};

	class Random
	{
		public:
			inline static void seed(i64 _seed) { Random::s_gen.seed(_seed); }
			inline static void autoSeed(void) { Random::s_gen.autoSeed(); }
			inline static i64 getSeed(void) { return Random::s_gen.getSeed(); }

			inline static f32 getf32(f32 min = 0.0f, f32 max = 1.0f) { return Random::s_gen.getf32(min, max); }
			inline static f64 getf64(f64 min = 0.0f, f64 max = 1.0f) { return Random::s_gen.getf64(min, max); }
			inline static i64 geti64(i64 min = LLONG_MIN, i64 max = LLONG_MAX) { return Random::s_gen.geti64(min, max); }
			inline static i32 geti32(i32 min = INT_MIN, i32 max = INT_MAX) { return Random::s_gen.geti32(min, max); }
			inline static i16 geti16(i16 min = SHRT_MIN, i16 max = SHRT_MAX) { return Random::s_gen.geti16(min, max); }
			inline static i8 geti8(i8 min = CHAR_MIN, i8 max = CHAR_MAX) { return Random::s_gen.geti8(min, max); }
			inline static u64 getui64(u64 min = 0, u64 max = ULLONG_MAX) { return Random::s_gen.getui64(min, max); }
			inline static u32 getui32(u32 min = 0, u32 max = UINT_MAX) { return Random::s_gen.getui32(min, max); }
			inline static u16 getui16(u16 min = 0, u16 max = USHRT_MAX) { return Random::s_gen.getui16(min, max); }
			inline static u8 getui8(u8 min = 0, u8 max = UCHAR_MAX) { return Random::s_gen.getui8(min, max); }
			inline static bool getb(f32 true_percentage = 0.5f) { return Random::s_gen.getb(true_percentage); }
			inline static Vec2 getVec2(f32 min = 0.0f, f32 max = 1.0f, bool match_xy = false) { return Random::s_gen.getVec2(min, max, match_xy); }
			inline static Vec2 getVec2(Vec2 minmax_x = { 0.0f, 1.0f }, Vec2 minmax_y = { 0.0f, 1.0f }) { return Random::s_gen.getVec2(minmax_x, minmax_y); }
			inline static f32 getOpenSimplex2D(f32 x, f32 y) { return Random::s_gen.getOpenSimplex2D(x, y); }

			template <typename T>
			inline static T& getFromStdVec(stdvec<T>& list) { return Random::s_gen.getFromStdVec<T>(list); }

		private:
			inline static RandomGenerator s_gen;
	};
}
