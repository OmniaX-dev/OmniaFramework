#ifndef __DATATYPES__HPP__
#define __DATATYPES__HPP__

#include <ostd/Types.hpp>

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

	class IOutputHandler;
	class Timer
	{
		public:
			inline Timer(void) { m_started = false; m_current = 0; m_timeUnit = eTimeUnits::Nanoseconds; m_dest = nullptr; }
			uint64_t start(bool print = true, String name = "", eTimeUnits timeUnit = eTimeUnits::Nanoseconds, IOutputHandler* __destination = nullptr);
			uint64_t end(bool print = true);

			static uint64_t getEpoch(eTimeUnits timeUnit = eTimeUnits::Milliseconds);

			inline const String& getName(void) const { return m_name; }

		private:
			bool m_started;
			int64_t m_current;
			eTimeUnits m_timeUnit;
			String m_name;
			IOutputHandler* m_dest;
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

	// class ColorInterpolator
	// {
	// 	public:
	// 		Color get(float percent);
	// 		inline void addColor(Color col) { m_colors.push_back(col); }

	// 	private:
	// 		std::vector<Color> m_colors;

	// };

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

	class IOutputHandler;
	class Utils
	{
		public:
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
			//TODO: Doesn't work on Windows
			//static bool setClipboardText(String text);
			//static String getClipboardText(void);
			static float map_value(float input, float input_start, float input_end, float output_start, float output_end);
			static bool loadFileFromHppResource(String output_file_path, const char* resource_buffer, unsigned int size);
			static void printByteStream(const ByteStream& data, StreamIndex start, uint8_t line_len, uint16_t n_rows, IOutputHandler& out, int32_t addrHighlight = -1, uint32_t highlightRange = 1, const String& title = "");
			static bool saveByteStreamToFile(const ByteStream& stream, const String& filePath);
			static bool loadByteStreamFromFile(const String& filePath, ByteStream& outStream);
			static ByteStream stringToByteStream(const String& data);
			static String byteStreamToString(const ByteStream& data);

			static std::vector<std::filesystem::path> listFilesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listFilesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoryRecursive(const String& directoryPath);
			
			static int32_t solveIntegerExpression(const String& expr);
			static void clearConsole(void);

		private:
			inline static uint64_t s_startTime_ms;
	};

	class StringEditor
	{
		public: class Tokens
		{
			public:
				inline bool hasNext(void) { return m_tokens.size() > 0 && m_current_index < m_tokens.size(); }
				inline bool hasPrevious(void) { return m_tokens.size() > 0 && m_current_index > 0; }
				String next(void);
				String previous(void);
				inline uint32_t count(void) { return m_tokens.size(); }
				inline std::vector<String> getRawData(void) { return m_tokens; }
				inline uint32_t getCurrentIndex(void) { return m_current_index; }
				inline void cycle(void) { m_current_index = 0; }

				inline auto begin(void) { return m_tokens.begin(); }
				inline auto end(void) { return m_tokens.end(); }
				inline auto cbegin(void) const { return m_tokens.begin(); }
				inline auto cend(void) const { return m_tokens.end(); }
				inline auto begin(void) const { return m_tokens.begin(); }
				inline auto end(void) const { return m_tokens.end(); }
				
			private:
				inline Tokens(void) { m_current_index = 0; }

			private:
				std::vector<String> m_tokens;
				uint32_t m_current_index;

			public:
				inline static const String END = "%END%";

				friend class StringEditor;
		};

		public:
			inline StringEditor(void) { m_data = ""; }
			inline StringEditor(const String& str) { m_data = str; }
			inline StringEditor(const char* str) { m_data = str; }
			inline String str(void) const { return m_data; }
			inline uint32_t len(void) { return m_data.length(); }
			inline StringEditor& clr(void) { m_data = ""; return *this; }
			inline const char* c_str(void) { return m_data.c_str(); }
			inline char at(uint32_t index) { return m_data[index]; }
			inline char operator[](uint32_t index) { return m_data[index]; }
			inline StringEditor& set(String str) { m_data = str; return *this; }

			StringEditor& ltrim(void);
			StringEditor& rtrim(void);
			StringEditor& trim(void);

			StringEditor& toLower(void);
			StringEditor& toUpper(void);

			StringEditor& addPadding(char c = ' ', uint32_t count = 10);
			//StringEditor& add(const StringEditor& se);
			StringEditor& add(String str);
			StringEditor& add(char c);
			StringEditor& addi(uint8_t i);
			StringEditor& addi(int8_t i);
			StringEditor& addi(uint16_t i);
			StringEditor& addi(int16_t i);
			StringEditor& addi(uint32_t i);
			StringEditor& addi(int32_t i);
			StringEditor& addi(uint64_t i);
			StringEditor& addi(int64_t i);
			StringEditor& addf(float f);
			StringEditor& addf(double f);

			StringEditor& reverse(void);
			StringEditor& replaceAll(String what, String with);
			StringEditor& replaceFirst(String what, String with);

			StringEditor& put(uint32_t index, char c);

			int64_t toInt(void);
			float toFloat(void);
			double toDouble(void);
			bool isNumeric(bool decimal = false);
			bool contains(char c);
			bool contains(String str);
			bool startsWith(String str);
			bool endsWith(String str);
			String getReverse(void);

			uint32_t count(String str);
			
			int32_t indexOf(char c, uint32_t start = 0);
			int32_t indexOf(String str, uint32_t start = 0);
			int32_t lastIndexOf(char c);
			int32_t lastIndexOf(String str);

			String substr(uint32_t start, int32_t end = -1);
			Tokens tokenize(String delimiter = " ", bool trim_tokens = true, bool allow_white_space_only_tokens = false);

			friend std::ostream& operator<<(std::ostream& out, const StringEditor& val);

		private:
			String m_data;
	};

	struct ConsoleCol
	{
		inline static constexpr const char* Red = "red";
		inline static constexpr const char* BrightRed = "b-red";
		inline static constexpr const char* OnRed = "o-red";
		inline static constexpr const char* OnBrightRed = "ob-red";
		
		inline static constexpr const char* Green = "green";
		inline static constexpr const char* BrightGreen = "b-green";
		inline static constexpr const char* OnGreen = "o-green";
		inline static constexpr const char* OnBrightGreen = "ob-green";
		
		inline static constexpr const char* Blue = "blue";
		inline static constexpr const char* BrightBlue = "b-blue";
		inline static constexpr const char* OnBlue = "o-blue";
		inline static constexpr const char* OnBrightBlue = "ob-blue";
		
		inline static constexpr const char* Magenta = "magenta";
		inline static constexpr const char* BrightMagenta = "b-magenta";
		inline static constexpr const char* OnMagenta = "o-magenta";
		inline static constexpr const char* OnBrightMagenta = "ob-magenta";
		
		inline static constexpr const char* Cyan = "cyan";
		inline static constexpr const char* BrightCyan = "b-cyan";
		inline static constexpr const char* OnCyan = "o-cyan";
		inline static constexpr const char* OnBrightCyan = "ob-cyan";
		
		inline static constexpr const char* Yellow = "yellow";
		inline static constexpr const char* BrightYellow = "b-yellow";
		inline static constexpr const char* OnYellow = "o-yellow";
		inline static constexpr const char* OnBrightYellow = "ob-yellow";
		
		inline static constexpr const char* Black = "gray";
		inline static constexpr const char* BrightGray = "b-gray";
		inline static constexpr const char* OnBlack = "o-gray";
		inline static constexpr const char* OnBrightGray = "ob-gray";
		
		inline static constexpr const char* Gray = "lgray";
		inline static constexpr const char* White = "white";
		inline static constexpr const char* onGray = "o-lgray";
		inline static constexpr const char* OnWhite = "ob-lgray";
	};

	class Color;
	class BaseObject;
	class IOutputHandler
	{
		public:
			virtual ~IOutputHandler(void) = default;
			virtual IOutputHandler& col(String color) = 0;
			virtual IOutputHandler& col(const Color& color) = 0;
			//virtual IOutputHandler& p(String str) = 0;
			virtual IOutputHandler& p(char c) = 0;
			virtual IOutputHandler& p(const StringEditor& se) = 0;
			//virtual IOutputHandler& p(const BaseObject& bo) = 0;
			virtual IOutputHandler& pi(uint8_t i) = 0;
			virtual IOutputHandler& pi(int8_t i) = 0;
			virtual IOutputHandler& pi(uint16_t i) = 0;
			virtual IOutputHandler& pi(int16_t i) = 0;
			virtual IOutputHandler& pi(uint32_t i) = 0;
			virtual IOutputHandler& pi(int32_t i) = 0;
			virtual IOutputHandler& pi(uint64_t i) = 0;
			virtual IOutputHandler& pi(int64_t i) = 0;
			virtual IOutputHandler& pf(float f, uint8_t precision = 0) = 0;
			virtual IOutputHandler& pf(double f, uint8_t precision = 0) = 0;
			virtual IOutputHandler& nl(void) = 0;
			virtual IOutputHandler& flush(void) = 0;
			virtual IOutputHandler& reset(void) = 0;

			virtual IOutputHandler& bgcol(const Color& color) = 0;
			virtual IOutputHandler& bgcol(String color) = 0;
			virtual IOutputHandler& resetColors(void) = 0;
			virtual IOutputHandler& clear(void) = 0;
	};

	class ConsoleOutputHandler : public IOutputHandler
	{
		public:
			IOutputHandler& col(String color) override;
			IOutputHandler& col(const Color& color) override;
			inline IOutputHandler& bgcol(const Color& color) override { return *this; }
			inline IOutputHandler& bgcol(String color) override { return *this; }
			inline IOutputHandler& resetColors(void) override { return *this; }
			//IOutputHandler& p(String str);
			IOutputHandler& p(char c) override;
			IOutputHandler& p(const StringEditor& se) override;
			//IOutputHandler& p(const BaseObject& bo);
			IOutputHandler& pi(uint8_t i) override;
			IOutputHandler& pi(int8_t i) override;
			IOutputHandler& pi(uint16_t i) override;
			IOutputHandler& pi(int16_t i) override;
			IOutputHandler& pi(uint32_t i) override;
			IOutputHandler& pi(int32_t i) override;
			IOutputHandler& pi(uint64_t i) override;
			IOutputHandler& pi(int64_t i) override;
			IOutputHandler& pf(float f, uint8_t precision = 0) override;
			IOutputHandler& pf(double f, uint8_t precision = 0) override;
			IOutputHandler& nl(void) override;
			IOutputHandler& flush(void) override;
			IOutputHandler& reset(void) override;
			IOutputHandler& clear(void) override;
	};

	class BufferedOutputHandler : public IOutputHandler
	{
		public:
			inline IOutputHandler& col(String color) override { return *this; }
			inline IOutputHandler& col(const Color& color) override { return *this; }
			inline IOutputHandler& bgcol(const Color& color) override { return *this; }
			inline IOutputHandler& bgcol(String color) override { return *this; }
			inline IOutputHandler& resetColors(void) override { return *this; }
			IOutputHandler& p(char c) override;
			IOutputHandler& p(const StringEditor& se) override;
			IOutputHandler& pi(uint8_t i) override;
			IOutputHandler& pi(int8_t i) override;
			IOutputHandler& pi(uint16_t i) override;
			IOutputHandler& pi(int16_t i) override;
			IOutputHandler& pi(uint32_t i) override;
			IOutputHandler& pi(int32_t i) override;
			IOutputHandler& pi(uint64_t i) override;
			IOutputHandler& pi(int64_t i) override;
			IOutputHandler& pf(float f, uint8_t precision = 0) override;
			IOutputHandler& pf(double f, uint8_t precision = 0) override;
			IOutputHandler& nl(void) override;
			inline IOutputHandler& flush(void) override { return *this; }
			inline IOutputHandler& reset(void) override { return *this; }

			inline IOutputHandler& clear(void) override { m_buffer = ""; return *this; }
			inline const StringEditor& getBuffer(void) { return m_buffer; }

		private:
			StringEditor m_buffer;
	};
} // namespace ox

#endif
