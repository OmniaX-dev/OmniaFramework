#ifndef __SERIAL_HPP__
#define __SERIAL_HPP__

#include <ostd/Types.hpp>
#include <ostd/Bitfields.hpp>
#include <ostd/BaseObject.hpp>
#include <unordered_map>

namespace ostd
{
	class IOutputHandler;

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
				//bool r_Data(StreamIndex addr, ISerializable& outData);
				bool r_ByteStream(StreamIndex addr, ByteStream& outStream);
				bool r_ByteStream(StreamIndex addr, ByteStream& outStream, uint32_t size);

				bool w_QWord(StreamIndex addr, QWord val);
				bool w_DWord(StreamIndex addr, DWord val);
				bool w_Word(StreamIndex addr, Word val);
				bool w_Byte(StreamIndex addr, Byte val);
				bool w_Addr(StreamIndex addr, StreamIndex val);
				bool w_Float(StreamIndex addr, float val);
				bool w_Double(StreamIndex addr, double val);
				//bool w_Data(StreamIndex addr, const ISerializable& data);
				bool w_ByteStream(StreamIndex addr, const ByteStream& stream);
				bool is_validAddr(StreamIndex addr, StreamIndex offset = 1);
				inline uint64_t size(void) { return m_data.size(); }
				inline ByteStream& getData(void) { return m_data; }

				void print(StreamIndex start, IOutputHandler& out);

				inline uint8_t getEndianness(void) const { return m_endianness; }
				inline bool isLittleEndian(void) const { return m_endianness == tEndianness::LittleEndian; }
				inline bool isBigEndian(void) const { return m_endianness == tEndianness::BigEndian; }

			private:
				ByteStream m_data;
				uint8_t m_endianness { tEndianness::BigEndian };

			public:
				inline static constexpr uint64_t DefaultMaxSize = 512;
		};

		/*struct tFieldType
		{
			inline static constexpr uint8_t Invalid = 0;
			inline static constexpr uint8_t Object = 0xFF;
			inline static constexpr uint8_t Stream = 0xFE;
			inline static constexpr uint8_t Array = 0xFD;
			inline static constexpr uint8_t Byte = 1;
			inline static constexpr uint8_t Word = 2;
			inline static constexpr uint8_t DWord = 4;
			inline static constexpr uint8_t QWord = 8;
			inline static constexpr uint8_t Float = 4;
			inline static constexpr uint8_t Double = 8;
		};

		class RawField
		{
			public:
				inline RawField(void) { m_type = tFieldType::Byte; m_data.push_back(0); }
				inline RawField(uint8_t type, ByteStream data = { 0 }) { m_type = type; m_data = data; }

				inline uint64_t size(void) const { return m_data.size(); }
				inline uint8_t getType(void) const { return m_type; }
				inline const ByteStream& getData(void) const { return m_data; }
				inline ByteStream& getDataRW(void) { return m_data; }
				inline void setType(uint8_t type) { m_type = type; }
				inline void setData(ByteStream data) { m_data = data; }

			protected:
				uint8_t m_type;
				ByteStream m_data;
		};

		class ObjectField
		{
			public: struct tFieldDescriptor
			{
				StreamIndex offset { 0 };
				uint8_t type { tFieldType::Invalid };
			};
			public:
				Byte getByte(String name);
				UByte getUByte(String name);
				Word getWord(String name);
				UWord getUWord(String name);
				DWord getDWord(String name);
				UDWord getUDWord(String name);
				QWord getQWord(String name);
				UQWord getUQWord(String name);
				float getFloat(String name);
				double getDouble(String name);
				ObjectField getObject(String name);

			private:
				ByteStream m_data;
				std::unordered_map<String, tFieldDescriptor> m_fields;

				friend class OXDBuilder;
		};

		class OXDBuilder
		{
			public:
				// static bool newFile(String path);
				// static bool writeFile(bool reset = true);
				// static bool addObject(ObjectField& object);
				// static void reset(void);

				static ByteStream stringToStream(String str);

			public: class Object
			{
				public:
					static bool create(String name, uint64_t oid = 0);
					static ObjectField get(void);
					static bool newField_Byte(ObjectField& obj, String name, Byte value);
					static bool newField_UByte(ObjectField& obj, String name, UByte value);
					static bool newField_Word(ObjectField& obj, String name, Word value);
					static bool newField_UWord(ObjectField& obj, String name, UWord value);
					static bool newField_DWord(ObjectField& obj, String name, DWord value);
					static bool newField_UDWord(ObjectField& obj, String name, UDWord value);
					static bool newField_QWord(ObjectField& obj, String name, QWord value);
					static bool newField_UQWord(ObjectField& obj, String name, UQWord value);
					static bool newField_Float(ObjectField& obj, String name, float value);
					static bool newField_Double(ObjectField& obj, String name, double value);
					static bool newField_Object(ObjectField& obj, String name, ObjectField& value);

				private:
					inline static ObjectField s_current = ObjectField();
					inline static bool s_open = false;
			};

			private:
				inline static String s_filePath = "";
				inline static bool s_open = false;
		};*/
	} //namespace serial
} // namespace ox



#endif
