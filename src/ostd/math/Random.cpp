#include "Random.hpp"
#include <chrono>

namespace ostd
{
	void RandomGenerator::seed(i64 _seed)
	{
		m_engine.seed(_seed);
		m_noiseGen.SetSeed(_seed);
		m_seed = _seed;
	}

	void RandomGenerator::autoSeed(void)
	{
		m_seed = std::chrono::steady_clock::now().time_since_epoch().count();
		m_engine.seed(m_seed);
		m_noiseGen.SetSeed(m_seed);
	}

	i64 RandomGenerator::getSeed(void)
	{
		return m_seed;
	}

	f32 RandomGenerator::getf32(f32 min, f32 max)
	{
		__swap_if_first_is_bigger<f32>(min, max);
		std::uniform_real_distribution<f32> dist(min, max);
		return dist(m_engine);
	}

	f64 RandomGenerator::getf64(f64 min, f64 max)
	{
		__swap_if_first_is_bigger<f64>(min, max);
		std::uniform_real_distribution<f64> dist(min, max);
		return dist(m_engine);
	}

	u64 RandomGenerator::getui64(u64 min, u64 max)
	{
		__swap_if_first_is_bigger<u64>(min, max);
		std::uniform_int_distribution<u64> dist(min, max);
		return dist(m_engine);
	}

	u32 RandomGenerator::getui32(u32 min, u32 max)
	{
		__swap_if_first_is_bigger<u32>(min, max);
		std::uniform_int_distribution<u32> dist(min, max);
		return dist(m_engine);
	}

	u16 RandomGenerator::getui16(u16 min, u16 max)
	{
		__swap_if_first_is_bigger<u16>(min, max);
		std::uniform_int_distribution<u16> dist(min, max);
		return dist(m_engine);
	}

	u8 RandomGenerator::getui8(u8 min, u8 max)
	{
		__swap_if_first_is_bigger<u8>(min, max);
		std::uniform_int_distribution<u8> dist(min, max);
		return dist(m_engine);
	}

	i64 RandomGenerator::geti64(i64 min, i64 max)
	{
		__swap_if_first_is_bigger<i64>(min, max);
		std::uniform_int_distribution<i64> dist(min, max);
		return dist(m_engine);
	}

	i32 RandomGenerator::geti32(i32 min, i32 max)
	{
		__swap_if_first_is_bigger<i32>(min, max);
		std::uniform_int_distribution<i32> dist(min, max);
		return dist(m_engine);
	}

	i16 RandomGenerator::geti16(i16 min, i16 max)
	{
		__swap_if_first_is_bigger<i16>(min, max);
		std::uniform_int_distribution<i16> dist(min, max);
		return dist(m_engine);
	}

	i8 RandomGenerator::geti8(i8 min, i8 max)
	{
		__swap_if_first_is_bigger<i8>(min, max);
		std::uniform_int_distribution<i8> dist(min, max);
		return dist(m_engine);
	}

	bool RandomGenerator::getb(f32 true_percentage)
	{
		return getf32() >= (1.0f - std::clamp(true_percentage, 0.0f, 1.0f));
	}

	Vec2 RandomGenerator::getVec2(f32 min, f32 max, bool match_xy)
	{
		f32 x = getf32(min, max);
		return { x, (match_xy ? x : getf32(min, max)) };
	}

	Vec2 RandomGenerator::getVec2(Vec2 minmax_x, Vec2 minmax_y)
	{
		return { getf32(minmax_x.x, minmax_x.y), getf32(minmax_y.x, minmax_y.y) };
	}

	f32 RandomGenerator::getOpenSimplex2D(f32 x, f32 y)
	{
		m_noiseGen.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
		return m_noiseGen.GetNoise(x, y);
	}

	String RandomGenerator::getString(u32 length, const String& charset)
	{
		String rnd = "";
		u32 len = charset.len();
		for (i32 i = 0; i < length; i++)
			rnd += charset[getui32(0, len - 1)];
		return rnd;
	}
}
