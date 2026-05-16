/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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

#include <ostd/string/String.hpp>
#include <span>

namespace ostd
{
	class __i_serializable;
	class Serial
	{
		public: template<typename T> struct ReadResult { T value; bool error { false }; };
		public: enum class Endianness : u8 { LittleEndian = 0, BigEndian };
		public: using StreamEdit = std::span<int8_t>;
		public: using StreamView = std::span<const int8_t>;
		public: class Stream
		{
			public:
				Stream(const Stream&) = delete;
				Stream& operator=(const Stream&) = delete;
				inline Stream(u64 size, Endianness endianness = Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0) {
					if (size < 1) return;
					if constexpr (sizeof(size_t) < sizeof(uint64_t))
					{
						if (size > std::numeric_limits<size_t>::max())
							throw "Serial::Stream size can't excede UINT32_MAX on a 32bit system.";
					}
					m_data = new i8[size];
					m_size = size;
					m_endianness = endianness;
					if (preInitialize)
					{
						for (u64 i = 0; i < size; i++)
							m_data[i] = initialValue;
					}
				}
				inline Stream(Stream&& other) noexcept : m_data(other.m_data), m_size(other.m_size) {
					other.m_data = nullptr;
					other.m_size = 0;
					m_endianness = other.m_endianness;
				}
				inline Stream& operator=(Stream&& other) noexcept {
					if (this != &other) {
						delete[] m_data;
						m_data = other.m_data;
						m_size = other.m_size;
						m_endianness = other.m_endianness;
						other.m_data = nullptr;
						other.m_size = 0;
					}
					return *this;
				}
				inline ~Stream(void) {
					delete[] m_data;
					m_data = nullptr;
				}
				inline bool isValid(void) const { return m_size > 0 && m_data != nullptr; }
				inline u64 size(void) const { return isValid() ? m_size : 0; }
				inline bool isLittleEndian(void) const { return m_endianness == Endianness::LittleEndian; }
				inline bool isBigEndian(void) const { return m_endianness == Endianness::BigEndian; }
				inline Endianness getEndianness(void) const { return m_endianness; }
				inline StreamEdit data() {
					if (isValid())
						return { m_data, cast<std::size_t>(m_size) };
					return {};
				}
				inline StreamView data() const {
					if (isValid())
						return { m_data, cast<std::size_t>(m_size) };
					return {};
				}

			private:
				i8* m_data { nullptr };
				u64 m_size { 0 };
				Endianness m_endianness { Endianness::LittleEndian };
		};

		public:
			inline static void attachStream(Stream& stream) { s_openStream = stream.data(); s_endianness = stream.getEndianness(); disableManagedAddress(); }
			inline static void detachStrean(void) { s_openStream = {}; }
			inline static bool setManagedAddress(u64 addr) { if (!isManagedAddressEnabled() || !isValidAddress(addr)) return false; s_addr = addr; return true; }
			inline static bool isManagedAddressEnabled(void) { return s_managedAddress; }
			inline static void enableManagedAddress(bool enable = true) { s_managedAddress = enable; s_addr = 0; }
			inline static void disableManagedAddress(void) { enableManagedAddress(false); }
			inline static bool isStreamAttached(void) { return !s_openStream.empty(); }
			inline static bool isLittleEndian(void) { return s_endianness == Endianness::LittleEndian; }
			inline static bool isBigEndian(void) { return s_endianness == Endianness::BigEndian; }
			static bool isValidAddress(u64 addr, u32 size = 1);

			public: struct read
			{
				ReadResult<i8> int8(u64 addr = 0);
				inline i8 int8d(u64 addr = 0, bool* outError = nullptr) { auto val = int8(addr); if (outError) *outError = val.error; return val.value; }
				ReadResult<u8> uint8(u64 addr = 0);
				inline u8 uint8d(u64 addr = 0, bool* outError = nullptr) { auto val = uint8(addr); if (outError) *outError = val.error; return val.value; }

				ReadResult<i16> int16(u64 addr = 0);
				inline i16 int16d(u64 addr = 0, bool* outError = nullptr) { auto val = int16(addr); if (outError) *outError = val.error; return val.value; }
				ReadResult<u16> uint16(u64 addr = 0);
				inline u16 uint16d(u64 addr = 0, bool* outError = nullptr) { auto val = uint16(addr); if (outError) *outError = val.error; return val.value; }

				ReadResult<i32> int32(u64 addr = 0);
				inline i32 int32d(u64 addr = 0, bool* outError = nullptr) { auto val = int32(addr); if (outError) *outError = val.error; return val.value; }
				ReadResult<u32> uint32(u64 addr = 0);
				inline u32 uint32d(u64 addr = 0, bool* outError = nullptr) { auto val = uint32(addr); if (outError) *outError = val.error; return val.value; }

				ReadResult<i64> int64(u64 addr = 0);
				inline i64 int64d(u64 addr = 0, bool* outError = nullptr) { auto val = int64(addr); if (outError) *outError = val.error; return val.value; }
				ReadResult<u64> uint64(u64 addr = 0);
				inline u64 uint64d(u64 addr = 0, bool* outError = nullptr) { auto val = uint64(addr); if (outError) *outError = val.error; return val.value; }

				ReadResult<f32> float32(u64 addr = 0);
				inline f32 float32d(u64 addr = 0, bool* outError = nullptr) { auto val = float32(addr); if (outError) *outError = val.error; return val.value; }
				ReadResult<f64> float64(u64 addr = 0);
				inline f64 float64d(u64 addr = 0, bool* outError = nullptr) { auto val = float64(addr); if (outError) *outError = val.error; return val.value; }

				bool stream(Stream& outStream, u64 addr = 0, bool sizeStored = true, u32 size_if_not_stored = 0);
				bool string(String& outString, u64 addr = 0, bool sizeStored = true, u32 size_if_not_stored = 0, bool null_terminated = false);
				bool object(__i_serializable& outObject, u64 addr = 0);

				private:
					read(void) = default;
			};

			public: struct write
			{
				bool int8(u64 addr, i8 value);
				inline bool int8(i8 value) { return int8(0, value); }
				bool uint8(u64 addr, u8 value);
				inline bool uint8(i8 value) { return uint8(0, value); }

				bool int16(u64 addr, i16 value);
				inline bool int16(i8 value) { return int16(0, value); }
				bool uint16(u64 addr, u16 value);
				inline bool uint16(i8 value) { return uint16(0, value); }

				bool int32(u64 addr, i32 value);
				inline bool int32(i8 value) { return int32(0, value); }
				bool uint32(u64 addr, u32 value);
				inline bool uint32(i8 value) { return uint32(0, value); }

				bool int64(u64 addr, i64 value);
				inline bool int64(i8 value) { return int64(0, value); }
				bool uint64(u64 addr, u64 value);
				inline bool uint64(i8 value) { return uint64(0, value); }

				bool float32(u64 addr, f32 value);
				inline bool float32(i8 value) { return float32(0, value); }
				bool float64(u64 addr, f64 value);
				inline bool float64(i8 value) { return float64(0, value); }

				bool stream(u64 addr, const Stream& value, bool storeSize = true);
				inline bool stream(const Stream& value, bool storeSize = true) { return stream(0, value, storeSize); }
				bool string(u64 addr, const String& value, bool storeSize = true, bool null_terminated = false); // if sizeStored, null_terminated will be ignored
				inline bool string(const String& value, bool storeSize = true, bool null_terminated = false) { return string(0, value, storeSize, null_terminated); }
				bool object(u64 addr, const __i_serializable& value);
				inline bool object(const __i_serializable& value) { return object(0, value); }

				private:
					write(void) = default;
			};

		private:
			Serial(void) = default;
			inline static void managed_address_check(u64& addr) { if (isManagedAddressEnabled()) addr = s_addr; }

		private:
			inline static StreamEdit s_openStream {  };
			inline static Endianness s_endianness { Endianness::LittleEndian };

			inline static u64 s_addr { 0 };
			inline static bool s_managedAddress { false };
	};
	class __i_serializable
	{
		public:
			virtual Serial::Stream serialize(void) const = 0;
			virtual bool deserialize(const Serial::Stream& data) = 0;
	};





	// =============================================== OLD VERSION ===============================================
	class OutputHandlerBase;
	namespace serial
	{
		class SerialIO
		{
			public: struct tEndianness {
				inline static constexpr u8 LittleEndian = 0x00;
				inline static constexpr u8 BigEndian = 0x01;
			};

			public:
				inline SerialIO(void) { init(); }
				inline SerialIO(u64 size, u8 endianness = tEndianness::BigEndian) { init(size, endianness); }
				inline SerialIO(ByteStream& data, u8 endianness = tEndianness::BigEndian) { m_data = data; m_endianness = endianness; }
				bool init(u64 size = SerialIO::DefaultMaxSize, u8 endianness = tEndianness::BigEndian);
				bool r_QWord(StreamIndex addr, QWord& outVal);
				bool r_DWord(StreamIndex addr, DWord& outVal);
				bool r_Word(StreamIndex addr, Word& outVal);
				bool r_Byte(StreamIndex addr, Byte& outVal);
				bool r_Addr(StreamIndex addr, StreamIndex& outVal);
				bool r_Float(StreamIndex addr, f32& outVal);
				bool r_Double(StreamIndex addr, f64& outVal);
				bool r_ByteStream(StreamIndex addr, ByteStream& outStream);
				bool r_ByteStream(StreamIndex addr, ByteStream& outStream, u32 size);
				bool r_String(StreamIndex addr, String& outString);
				bool r_String(StreamIndex addr, String& outString, u32 size);
				bool r_NullTerminatedString(StreamIndex addr, String& outString);

				bool w_QWord(StreamIndex addr, QWord val);
				bool w_DWord(StreamIndex addr, DWord val);
				bool w_Word(StreamIndex addr, Word val);
				bool w_Byte(StreamIndex addr, Byte val);
				bool w_Addr(StreamIndex addr, StreamIndex val);
				bool w_Float(StreamIndex addr, f32 val);
				bool w_Double(StreamIndex addr, f64 val);
				bool w_ByteStream(StreamIndex addr, const ByteStream& stream, bool store_size = true);
				bool w_String(StreamIndex addr, const String& str, bool store_size = false, bool null_terminate = true);
				bool is_validAddr(StreamIndex addr, StreamIndex offset = 1);
				inline u64 size(void) { return m_data.size(); }
				inline ByteStream& getData(void) { return m_data; }

				void print(StreamIndex start, OutputHandlerBase& out);
				bool saveToFile(const String& filePath);

				inline u8 getEndianness(void) const { return m_endianness; }
				inline bool isLittleEndian(void) const { return m_endianness == tEndianness::LittleEndian; }
				inline bool isBigEndian(void) const { return m_endianness == tEndianness::BigEndian; }

				inline bool isAutoResizeEnabled(void) const { return m_autoResize; }
				inline void enableAutoResize(bool val = true, i32 resizeAmount = 1024) { m_autoResize = val; if (!val) return; m_resizeAmount = resizeAmount; }

			private:
				ByteStream m_data;
				u8 m_endianness { tEndianness::BigEndian };
				bool m_autoResize { true };
				i32 m_resizeAmount { 1024 };
				bool m_statuWriting { false };

			public:
				inline static constexpr u64 DefaultMaxSize = 512;
		};
	}
}
