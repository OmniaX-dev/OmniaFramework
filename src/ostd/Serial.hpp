#ifndef __SERIAL_HPP__
#define __SERIAL_HPP__

#include <ostd/Types.hpp>
#include <ostd/Bitfields.hpp>
#include <ostd/BaseObject.hpp>
#include <unordered_map>

namespace ostd
{
	class OutputHandlerBase;

	namespace serial
	{
		class SerialIO
		{
			public: struct tEndianness {
				inline static constexpr uint8_t LittleEndian = 0x00;
				inline static constexpr uint8_t BigEndian = 0x01;
			};

			public:
				inline SerialIO(void) { init(); }
				inline SerialIO(uint64_t size, uint8_t endianness = tEndianness::BigEndian) { init(size, endianness); }
				inline SerialIO(ByteStream& data, uint8_t endianness = tEndianness::BigEndian) { m_data = data; m_endianness = endianness; }
				bool init(uint64_t size = SerialIO::DefaultMaxSize, uint8_t endianness = tEndianness::BigEndian);
				bool r_QWord(StreamIndex addr, QWord& outVal);
				bool r_DWord(StreamIndex addr, DWord& outVal);
				bool r_Word(StreamIndex addr, Word& outVal);
				bool r_Byte(StreamIndex addr, Byte& outVal);
				bool r_Addr(StreamIndex addr, StreamIndex& outVal);
				bool r_Float(StreamIndex addr, float& outVal);
				bool r_Double(StreamIndex addr, double& outVal);
				bool r_ByteStream(StreamIndex addr, ByteStream& outStream);
				bool r_ByteStream(StreamIndex addr, ByteStream& outStream, uint32_t size);
				bool r_String(StreamIndex addr, String& outString);
				bool r_String(StreamIndex addr, String& outString, uint32_t size);
				bool r_NullTerminatedString(StreamIndex addr, String& outString);

				bool w_QWord(StreamIndex addr, QWord val);
				bool w_DWord(StreamIndex addr, DWord val);
				bool w_Word(StreamIndex addr, Word val);
				bool w_Byte(StreamIndex addr, Byte val);
				bool w_Addr(StreamIndex addr, StreamIndex val);
				bool w_Float(StreamIndex addr, float val);
				bool w_Double(StreamIndex addr, double val);
				bool w_ByteStream(StreamIndex addr, const ByteStream& stream, bool store_size = true);
				bool w_String(StreamIndex addr, const String& str, bool store_size = false, bool null_terminate = true);
				bool is_validAddr(StreamIndex addr, StreamIndex offset = 1);
				inline uint64_t size(void) { return m_data.size(); }
				inline ByteStream& getData(void) { return m_data; }

				void print(StreamIndex start, OutputHandlerBase& out);
				bool saveToFile(const String& filePath);

				inline uint8_t getEndianness(void) const { return m_endianness; }
				inline bool isLittleEndian(void) const { return m_endianness == tEndianness::LittleEndian; }
				inline bool isBigEndian(void) const { return m_endianness == tEndianness::BigEndian; }

				inline bool isAutoResizeEnabled(void) const { return m_autoResize; }
				inline void enableAutoResize(bool val = true, int32_t resizeAmount = 1024) { m_autoResize = val; if (!val) return; m_resizeAmount = resizeAmount; }

			private:
				ByteStream m_data;
				uint8_t m_endianness { tEndianness::BigEndian };
				bool m_autoResize { true };
				int32_t m_resizeAmount { 1024 };
				bool m_statuWriting { false };

			public:
				inline static constexpr uint64_t DefaultMaxSize = 512;
		};
	} //namespace serial
} // namespace ostd

#endif
