#ifndef __DATATYPES__HPP__
#define __DATATYPES__HPP__

#include <cstdint>
#include <ostd/Types.hpp>
#include <ostd/TextStyleParser.hpp>
#include <ostd/Defines.hpp>

#include <filesystem>

namespace ostd
{
	enum class eTimeUnits
	{
		Seconds = 0,
		Milliseconds,
		Microseconds,
		Nanoseconds
	};
	enum class eMonths
	{
		January = 0,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};


	template <class T>
	class _Counter
	{
		public:
			inline _Counter(void) { m_count = 200; m_current = 0; }
			inline _Counter(T count, T step = 1) { m_count = count; m_current = 0; m_step = step; }
			inline void setCount(T count) { m_count = count; m_counting = false; m_current = 0; }
			inline T getCount(void) { return m_count; }
			inline T getCurrent(void) { return m_current; };
			inline T getStep(void) { return m_step; }
			inline void start(void) { m_current = 0; m_counting = true; }
			inline void stop(void) { m_current = 0; m_counting = false; }
			inline bool isDone(void) { return !m_counting; }
			inline bool isCounting(void) { return m_counting; }
			inline void update(void)
			{
				if (!m_counting) return;
				m_current += m_step;
				if (m_current > m_count)
					stop();
			}

		private:
			T m_current { 0 };
			T m_count { 200 };
			T m_step { 1 };
			bool m_counting { false };
	};

	typedef _Counter<uint64_t> Counter;
	typedef _Counter<float> FloatCounter;
	typedef _Counter<double> DoubleCounter;

	class OutputHandlerBase;
	class Timer
	{
		public:
			inline Timer(void) { m_started = false; m_current = 0; m_timeUnit = eTimeUnits::Nanoseconds; m_dest = nullptr; }
			uint64_t start(bool print = true, String name = "", eTimeUnits timeUnit = eTimeUnits::Nanoseconds, OutputHandlerBase* __destination = nullptr);
			uint64_t startCount(eTimeUnits timeUnit = eTimeUnits::Nanoseconds);
			uint64_t end(bool print = true);
			uint64_t endCount(bool stop = true);

			static uint64_t getEpoch(eTimeUnits timeUnit = eTimeUnits::Milliseconds);

			inline const String& getName(void) const { return m_name; }
			inline uint64_t read(void) { if (!m_started) return 0; return endCount(false); }

		private:
			bool m_started;
			int64_t m_current;
			eTimeUnits m_timeUnit;
			String m_name;
			OutputHandlerBase* m_dest;
	};

	struct GameClock
	{
		public:
			GameClock(void);
			const float& start(void);
			inline const float& getTimeOfDay(void) const { return m_timeOfDay; }
			String asString(void);
			void update(void);

		private:
			String getFormattedTime(void);
			String convertMonth(void);

		public:
			uint8_t minutes;
			uint8_t hours;
			uint16_t days;
			uint8_t months;
			uint16_t years;

		private:
			Timer m_rtClock;
			float m_timeOfDay;
			float m_totalSeconds;
	};

	class LocalTime
	{
		public:
			String getFullString(bool include_date = true, bool include_time = true, bool day_name = true, bool month_as_name = true, bool include_seconds = true) const;
			inline String getDateString(bool day_name = true, bool month_as_name = true) { return getFullString(true, false, day_name, month_as_name, false); }
			inline String getTimeString(bool include_seconds = true) { return getFullString(false, true, false, false, include_seconds); }
			int32_t hours(void) const;
			int32_t minutes(void) const;
			int32_t seconds(void) const;
			int32_t day(void) const;
			int32_t month(void) const;
			int32_t year(void) const;
			int32_t weekDay(void) const;
			String shours(bool leading_zero = true) const;
			String sminutes(bool leading_zero = true) const;
			String sseconds(bool leading_zero = true) const;
			String sday(bool leading_zero = true) const;
			String smonth(bool leading_zero = true, bool month_name = true) const;
			String syear(void) const;
			String sWeekDay(bool day_name = true) const;

		protected:
			virtual String monthToText(int32_t month) const;
			virtual String weekDayToText(int32_t day) const;
	};
	typedef LocalTime LocalTime_EN;
	class LocalTime_IT : public LocalTime
	{
		protected:
			String monthToText(int32_t month) const override;
			String weekDayToText(int32_t day) const override;
	};
	class LocalTime_ES : public LocalTime
	{
		protected:
			String monthToText(int32_t month) const override;
			String weekDayToText(int32_t day) const override;
	};
	class LocalTime_DE : public LocalTime
	{
		protected:
			String monthToText(int32_t month) const override;
			String weekDayToText(int32_t day) const override;
	};

	class Utils
	{
		public:
			//Implemented in <Utils.cpp>
			static inline const uint64_t getStartTime(void) { return Utils::s_startTime_ms; }
			static void init(void);
			static bool isHex(String hex);
			static bool isBin(String bin);
			static bool isInt(String str);
			static int64_t strToInt(String str);
			static bool readFile(String fileName, std::vector<String>& outLines);
			static String getHexStr(uint64_t value, bool prefix = true, uint8_t nbytes = 1);
			static String getBinStr(uint64_t value, bool prefix = true, uint8_t nbytes = 1);
			static String duplicateChar(unsigned char c, uint16_t count);
			static void sleep(uint32_t __time, eTimeUnits __unit = eTimeUnits::Milliseconds);
			static uint64_t getRunningTime_ms(void);
			static float map_value(float input, float input_start, float input_end, float output_start, float output_end);
			static bool loadFileFromHppResource(String output_file_path, const char* resource_buffer, unsigned int size);
			static void printByteStream(const ByteStream& data, StreamIndex start, uint8_t line_len, uint16_t n_rows, OutputHandlerBase& out, int32_t addrHighlight = -1, uint32_t highlightRange = 1, const String& title = "");
			static bool saveByteStreamToFile(const ByteStream& stream, const String& filePath);
			static bool loadByteStreamFromFile(const String& filePath, ByteStream& outStream);
			static ByteStream stringToByteStream(const String& data);
			static String byteStreamToString(const ByteStream& data);

			//Implemented in <md5.cpp>
			static String md5(const String& str);

			//Implemented in <FileSystem.cpp>
			static std::vector<std::filesystem::path> listFilesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listFilesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoryRecursive(const String& directoryPath);
			static ostd::String getHomeDirPath(void);
			static std::filesystem::path getWorkingDirPath(void);

			//Implemented in <ShuntingYard.cpp>
			static int32_t solveIntegerExpression(const String& expr);

			//Implemented in <Console.cpp>
			static void clearConsole(void);
			static void getConsoleSize(int32_t& outColumns, int32_t& outRows);
			static void setConsoleCursorPosition(int32_t x, int32_t y);
			static int32_t getConsoleWidth(void);
			static int32_t getConsoleHeight(void);

			//Array helpers
			template<typename T>
			static inline T* createArray(size_t size)
			{
				T* array = (T*)malloc(size * sizeof(T));
				return array;
			}
			template<typename T>
			static inline T* resizeArray(T* array, size_t new_size)
			{
				T* new_array = (T*)realloc(array, new_size * sizeof(T));
				return new_array;
			}
			static inline void destroyArray(void* array)
			{
				free(array);
			}

		private:
			static void transform(const uint8_t block[64], uint32_t state[4]);
			static void encode(uint8_t* output, const uint32_t* input, size_t len);
			static void decode(uint32_t* output, const uint8_t* input, size_t len);

		private:
			inline static uint64_t s_startTime_ms;
	};
}

#endif
