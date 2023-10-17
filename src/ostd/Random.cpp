#include "Random.hpp"
#include <chrono>

namespace ostd
{
	void RandomGenerator::seed(int64_t _seed)
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
	
	int64_t RandomGenerator::getSeed(void)
	{
		return m_seed;
	}
	
	float RandomGenerator::getf32(float min, float max)
	{
		std::uniform_real_distribution<float> dist(min, max);
		return dist(m_engine);
	}
	
	double RandomGenerator::getf64(double min, double max)
	{
		std::uniform_real_distribution<double> dist(min, max);
		return dist(m_engine);
	}
	
	uint64_t RandomGenerator::getui64(uint64_t min, uint64_t max)
	{
		std::uniform_int_distribution<uint64_t> dist(min, max);
		return dist(m_engine);
	}
	
	uint32_t RandomGenerator::getui32(uint32_t min, uint32_t max)
	{
		std::uniform_int_distribution<uint32_t> dist(min, max);
		return dist(m_engine);
	}
	
	uint16_t RandomGenerator::getui16(uint16_t min, uint16_t max)
	{
		std::uniform_int_distribution<uint16_t> dist(min, max);
		return dist(m_engine);
	}
	
	uint8_t RandomGenerator::getui8(uint8_t min, uint8_t max)
	{
		std::uniform_int_distribution<uint8_t> dist(min, max);
		return dist(m_engine);
	}
	
	int64_t RandomGenerator::geti64(int64_t min, int64_t max)
	{
		std::uniform_int_distribution<int64_t> dist(min, max);
		return dist(m_engine);
	}
	
	int32_t RandomGenerator::geti32(int32_t min, int32_t max)
	{
		std::uniform_int_distribution<int32_t> dist(min, max);
		return dist(m_engine);
	}
	
	int16_t RandomGenerator::geti16(int16_t min, int16_t max)
	{
		std::uniform_int_distribution<int16_t> dist(min, max);
		return dist(m_engine);
	}
	
	int8_t RandomGenerator::geti8(int8_t min, int8_t max)
	{
		std::uniform_int_distribution<int8_t> dist(min, max);
		return dist(m_engine);
	}
	
	bool RandomGenerator::getb(float true_percentage)
	{
		return getf32() >= (1.0f - std::clamp(true_percentage, 0.0f, 1.0f));
	}
	
	Vec2 RandomGenerator::getVec2(float min, float max, bool match_xy)
	{
		float x = getf32(min, max);
		return { x, (match_xy ? x : getf32(min, max)) };
	}
	
	Vec2 RandomGenerator::getVec2(Vec2 minmax_x, Vec2 minmax_y)
	{
		return { getf32(minmax_x.x, minmax_x.y), getf32(minmax_y.x, minmax_y.y) };
	}

	float RandomGenerator::getOpenSimplex2D(float x, float y)
	{
		m_noiseGen.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
		return m_noiseGen.GetNoise(x, y);
	}
	
}
