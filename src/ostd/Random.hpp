#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

#include <random>
#include <climits>
#include <ostd/Geometry.hpp>
#include <ostd/vendor/FastNoiseLite.hpp>

namespace ostd
{
	class RandomGenerator
	{
		public:
			inline RandomGenerator(bool auto_seed = true) { if (auto_seed) autoSeed(); }
			inline RandomGenerator(int64_t _seed) { seed(_seed); }

			void seed(int64_t _seed);
			void autoSeed(void);
			int64_t getSeed(void);

			float getf32(float min = 0.0f, float max = 1.0f);
			double getf64(double min = 0.0f, double max = 1.0f);
			int64_t geti64(int64_t min = LLONG_MIN, int64_t max = LLONG_MAX);
			int32_t geti32(int32_t min = INT_MIN, int32_t max = INT_MAX);
			int16_t geti16(int16_t min = SHRT_MIN, int16_t max = SHRT_MAX);
			int8_t geti8(int8_t min = CHAR_MIN, int8_t max = CHAR_MAX);
			uint64_t getui64(uint64_t min = 0, uint64_t max = ULLONG_MAX);
			uint32_t getui32(uint32_t min = 0, uint32_t max = UINT_MAX);
			uint16_t getui16(uint16_t min = 0, uint16_t max = USHRT_MAX);
			uint8_t getui8(uint8_t min = 0, uint8_t max = UCHAR_MAX);
			bool getb(float true_percentage = 0.5f);
			Vec2 getVec2(float min = 0.0f, float max = 1.0f, bool match_xy = false);
			Vec2 getVec2(Vec2 minmax_x = { 0.0f, 1.0f }, Vec2 minmax_y = { 0.0f, 1.0f });
			float getOpenSimplex2D(float x, float y);

			inline FastNoiseLite& getNoiseGenerator(void) { return m_noiseGen; }

			template <typename T>
			inline T& getFromStdVec(std::vector<T>& list)
			{
				uint64_t index = static_cast<uint64_t>(geti64(0, list.size()));
				return list[index];
			}

		private:
			int64_t m_seed { 0 };
			std::mt19937_64 m_engine;
			FastNoiseLite m_noiseGen;
	};

	class Random
	{
		public:
			inline static void seed(int64_t _seed) { Random::s_gen.seed(_seed); }
			inline static void autoSeed(void) { Random::s_gen.autoSeed(); }
			inline static int64_t getSeed(void) { return Random::s_gen.getSeed(); }

			inline static float getf32(float min = 0.0f, float max = 1.0f) { return Random::s_gen.getf32(min, max); }
			inline static double getf64(double min = 0.0f, double max = 1.0f) { return Random::s_gen.getf64(min, max); }
			inline static int64_t geti64(int64_t min = LLONG_MIN, int64_t max = LLONG_MAX) { return Random::s_gen.geti64(min, max); }
			inline static int32_t geti32(int32_t min = INT_MIN, int32_t max = INT_MAX) { return Random::s_gen.geti32(min, max); }
			inline static int16_t geti16(int16_t min = SHRT_MIN, int16_t max = SHRT_MAX) { return Random::s_gen.geti16(min, max); }
			inline static int8_t geti8(int8_t min = CHAR_MIN, int8_t max = CHAR_MAX) { return Random::s_gen.geti8(min, max); }
			inline static uint64_t getui64(uint64_t min = 0, uint64_t max = ULLONG_MAX) { return Random::s_gen.getui64(min, max); }
			inline static uint32_t getui32(uint32_t min = 0, uint32_t max = UINT_MAX) { return Random::s_gen.getui32(min, max); }
			inline static uint16_t getui16(uint16_t min = 0, uint16_t max = USHRT_MAX) { return Random::s_gen.getui16(min, max); }
			inline static uint8_t getui8(uint8_t min = 0, uint8_t max = UCHAR_MAX) { return Random::s_gen.getui8(min, max); }
			inline static bool getb(float true_percentage = 0.5f) { return Random::s_gen.getb(true_percentage); }
			inline static Vec2 getVec2(float min = 0.0f, float max = 1.0f, bool match_xy = false) { return Random::s_gen.getVec2(min, max, match_xy); }
			inline static Vec2 getVec2(Vec2 minmax_x = { 0.0f, 1.0f }, Vec2 minmax_y = { 0.0f, 1.0f }) { return Random::s_gen.getVec2(minmax_x, minmax_y); }
			inline static float getOpenSimplex2D(float x, float y) { return Random::s_gen.getOpenSimplex2D(x, y); }

			template <typename T>
			inline static T& getFromStdVec(std::vector<T>& list) { return Random::s_gen.getFromStdVec<T>(list); }

		private:
			inline static RandomGenerator s_gen;
	};
}

#endif
