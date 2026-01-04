#ifndef __DATATYPES__HPP__
#define __DATATYPES__HPP__

#include <cstdint>
#include <ostd/Types.hpp>
#include <ostd/TextStyleParser.hpp>
#include <ostd/Defines.hpp>
#include <ostd/Time.hpp>

#include <filesystem>

namespace ostd
{
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
			static float map_value(float input, float input_start, float input_end, float output_start, float output_end);
			static bool loadFileFromHppResource(String output_file_path, const char* resource_buffer, unsigned int size);
			static void printByteStream(const ByteStream& data, StreamIndex start, uint8_t line_len, uint16_t n_rows, OutputHandlerBase& out, int32_t addrHighlight = -1, uint32_t highlightRange = 1, const String& title = "");
			static bool saveByteStreamToFile(const ByteStream& stream, const String& filePath);
			static bool loadByteStreamFromFile(const String& filePath, ByteStream& outStream);
			static ByteStream stringToByteStream(const String& data);
			static String byteStreamToString(const ByteStream& data);

			//Implemented in <Time.hpp>
			static void sleep(uint32_t __time, eTimeUnits __unit = eTimeUnits::Milliseconds);
			static uint64_t getRunningTime_ms(void);
			static String secondsToFormattedString(int32_t totalSeconds);

			//Implemented in <md5.cpp>
			static String md5(const String& str);

			//Implemented in <FileSystem.cpp>
			static std::vector<std::filesystem::path> listFilesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listFilesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoryRecursive(const String& directoryPath);
			static std::filesystem::path getHomeDirPath(void);
			static std::filesystem::path getWorkingDirPath(void);
			static bool ensureDirectory(const String& path);
			static bool deleteDirectory(const String& path);

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
