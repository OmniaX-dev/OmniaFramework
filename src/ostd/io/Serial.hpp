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
	class I_serializable;
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
				inline Stream(void) {  } // Uninitialized Stream
				inline Stream(u64 size, Endianness endianness = Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0) {
					construct(size, endianness, preInitialize, initialValue);
				}
				inline Stream(Stream&& other) noexcept : m_data(other.m_data), m_size(other.m_size), m_endianness(other.m_endianness) {
					other.m_data = nullptr;
					other.m_size = 0;
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
				inline bool construct(u64 size, Endianness endianness = Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0) {
					if (isValid()) return false;
					if (size < 1) return false;
					if constexpr (sizeof(size_t) < sizeof(uint64_t))
					{
						if (size > std::numeric_limits<size_t>::max())
							throw std::runtime_error("Serial::Stream size can't excede UINT32_MAX on a 32bit system.");
					}
					m_data = new i8[size];
					m_size = size;
					m_endianness = endianness;
					if (preInitialize)
					{
						for (u64 i = 0; i < size; i++)
							m_data[i] = initialValue;
					}
					return true;
				}
				inline bool isValid(void) const { return m_size > 0 && m_data != nullptr; }
				inline u64 size(void) const { return isValid() ? m_size : 0; }
				inline bool isLittleEndian(void) const { return m_endianness == Endianness::LittleEndian; }
				inline bool isBigEndian(void) const { return m_endianness == Endianness::BigEndian; }
				inline Endianness getEndianness(void) const { return m_endianness; }
				inline void setEndianness(Endianness endianness) { m_endianness = endianness; }
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
		private: struct Reader
		{
			inline Reader(Serial& _parent) : parent(_parent) { data = parent.m_data.data(); }

			ReadResult<i8> int8(u64 addr);
			inline ReadResult<i8> int8(void) { return int8(0); }
			ReadResult<u8> uint8(u64 addr);
			inline ReadResult<u8> uint8(void) { return uint8(0); }

			ReadResult<i16> int16(u64 addr);
			inline ReadResult<i16> int16(void) { return int16(0); }
			ReadResult<u16> uint16(u64 addr);
			inline ReadResult<u16> uint16(void) { return uint16(0); }

			ReadResult<i32> int32(u64 addr);
			inline ReadResult<i32> int32(void) { return int32(0); }
			ReadResult<u32> uint32(u64 addr);
			inline ReadResult<u32> uint32(void) { return uint32(0); }

			ReadResult<i64> int64(u64 addr);
			inline ReadResult<i64> int64(void) { return int64(0); }
			ReadResult<u64> uint64(u64 addr);
			inline ReadResult<u64> uint64(void) { return uint64(0); }

			ReadResult<f32> float32(u64 addr);
			inline ReadResult<f32> float32(void) { return float32(0); }
			ReadResult<f64> float64(u64 addr);
			inline ReadResult<f64> float64(void) { return float64(0); }

			bool stream(u64 addr, Stream& outStream, bool sizeStored = true, u32 size_if_not_stored = 0);
			inline bool stream(Stream& outStream, bool sizeStored = true, u32 size_if_not_stored = 0) { return stream(0, outStream, sizeStored, size_if_not_stored); }
			bool string(u64 addr, String& outString, bool sizeStored = true, u32 size_if_not_stored = 0, bool null_terminated = false);
			inline bool string(String& outString, bool sizeStored = true, u32 size_if_not_stored = 0, bool null_terminated = false) { return string(0, outString, sizeStored, size_if_not_stored, null_terminated); }
			bool object(u64 addr, I_serializable& outObject);
			inline bool object(I_serializable& outObject) { return object(0, outObject); }

			private: // Convenience wrappers
				inline bool isValid(void) const { return parent.isValid(); }
				inline bool isInvalid(void) const { return parent.isInvalid(); }
				inline bool isLittleEndian(void) const { return parent.isLittleEndian(); }
				inline bool isBigEndian(void) const { return parent.isBigEndian(); }
				inline u64& resolve_address(u64& addr) { return parent.resolve_address(addr); }
				inline bool isValidAddress(u64 addr, u32 size = 1) const { return parent.isValidAddress(addr); }

			private:
				Serial& parent;
				StreamView data;
		};
		private: struct Writer
		{
			inline Writer(Serial& _parent) : parent(_parent) { data = parent.m_data.data(); }

			bool int8(u64 addr, i8 value);
			inline bool int8(i8 value) { return int8(0, value); }
			bool uint8(u64 addr, u8 value);
			inline bool uint8(u8 value) { return uint8(0, value); }

			bool int16(u64 addr, i16 value);
			inline bool int16(i16 value) { return int16(0, value); }
			bool uint16(u64 addr, u16 value);
			inline bool uint16(u16 value) { return uint16(0, value); }

			bool int32(u64 addr, i32 value);
			inline bool int32(i32 value) { return int32(0, value); }
			bool uint32(u64 addr, u32 value);
			inline bool uint32(u32 value) { return uint32(0, value); }

			bool int64(u64 addr, i64 value);
			inline bool int64(i64 value) { return int64(0, value); }
			bool uint64(u64 addr, u64 value);
			inline bool uint64(u64 value) { return uint64(0, value); }

			bool float32(u64 addr, f32 value);
			inline bool float32(f32 value) { return float32(0, value); }
			bool float64(u64 addr, f64 value);
			inline bool float64(f64 value) { return float64(0, value); }

			bool stream(u64 addr, const Stream& value, bool storeSize = true);
			inline bool stream(const Stream& value, bool storeSize = true) { return stream(0, value, storeSize); }
			bool string(u64 addr, const String& value, bool storeSize = true, bool null_terminated = false); // if sizeStored, null_terminated will be ignored
			inline bool string(const String& value, bool storeSize = true, bool null_terminated = false) { return string(0, value, storeSize, null_terminated); }
			bool object(u64 addr, const I_serializable& value);
			inline bool object(const I_serializable& value) { return object(0, value); }

			private: // Convenience wrappers
				inline bool isValid(void) const { return parent.isValid(); }
				inline bool isInvalid(void) const { return parent.isInvalid(); }
				inline bool isLittleEndian(void) const { return parent.isLittleEndian(); }
				inline bool isBigEndian(void) const { return parent.isBigEndian(); }
				inline u64& resolve_address(u64& addr) { return parent.resolve_address(addr); }
				inline bool isValidAddress(u64 addr, u32 size = 1) const { return parent.isValidAddress(addr); }

			private:
				Serial& parent;
				StreamEdit data;
		};

		public:
			inline Serial(void) : read(*this), write(*this) { }
			inline Serial(u64 size, Endianness endianness = Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0) : read(*this), write(*this) { construct(size, endianness, preInitialize, initialValue); }
			bool construct(u64 size, Endianness endianness = Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0);
			bool resetAddress(u64 newAddr);
			bool isValidAddress(u64 addr, u32 size = 1) const;

			inline bool isValid(void) const { return m_data.isValid(); }
			inline bool isInvalid(void) const { return !isValid(); }
			inline bool isLittleEndian(void) const { return m_data.isLittleEndian(); }
			inline bool isBigEndian(void) const { return m_data.isBigEndian(); }
			inline Endianness getEndianness(void) const { return m_data.getEndianness(); }

		private:
			inline u64& resolve_address(u64& addr) { if (addr == 0) return m_address; return addr; }
			inline bool restore_address(void) { resetAddress(m_backupAddress); return false; } // Always returns false so it can be used as false on error in return statements

		private:
			Stream m_data;
			u64 m_address { 0 };
			u64 m_backupAddress { 0 };
			bool m_backupStored { false };

		public:
			Writer write;
			Reader read;
	};
	class I_serializable
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
