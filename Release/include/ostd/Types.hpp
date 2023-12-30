#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <string>
#include <cstdint>
#include <vector>

namespace ostd
{
	namespace legacy
	{
		typedef std::string String;
	}
	typedef std::string cpp_string;

	typedef int64_t QWord;
	typedef int32_t DWord;
	typedef int16_t Word;
	typedef int8_t Byte;

	typedef uint64_t UQWord;
	typedef uint32_t UDWord;
	typedef uint16_t UWord;
	typedef uint8_t UByte;

	typedef uint32_t StreamIndex;

	typedef union {
		float val;
		Byte data[4];
		int32_t raw;
	} __float_parser;
	typedef union {
		double val;
		Byte data[8];
		int64_t raw;
	} __double_parser;

	struct tTypeSize
	{
		static inline const uint8_t BYTE = 1;
		static inline const uint8_t WORD = 2;
		static inline const uint8_t DWORD = 4;
		static inline const uint8_t QWORD = 8;
		static inline const uint8_t ADDR = 4;
		static inline const uint8_t FLOAT = 4;
		static inline const uint8_t DOUBLE = 8;
	};

	typedef std::vector<Byte> ByteStream;
} 



#endif
