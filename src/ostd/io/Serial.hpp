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
	namespace serial
	{
		enum class Endianness : u8 { LittleEndian = 0, BigEndian };


		// ============================================================
		// Byte-packing utilities
		// ============================================================

		// Map a type to its unsigned-integer "bit container" of the same size.
		namespace detail
		{
			template<typename T> struct bit_container;
			template<> struct bit_container<i8>  { using type = u8;  };
			template<> struct bit_container<u8>  { using type = u8;  };
			template<> struct bit_container<i16> { using type = u16; };
			template<> struct bit_container<u16> { using type = u16; };
			template<> struct bit_container<i32> { using type = u32; };
			template<> struct bit_container<u32> { using type = u32; };
			template<> struct bit_container<i64> { using type = u64; };
			template<> struct bit_container<u64> { using type = u64; };
			template<> struct bit_container<f32> { using type = u32; };
			template<> struct bit_container<f64> { using type = u64; };

			template<typename T>
			using bit_container_t = typename bit_container<T>::type;
		}

		// Pack any 1/2/4/8-byte scalar into an array of bytes.
		// Works for signed and unsigned integers, and for floats.
		template<typename T>
		std::array<i8, sizeof(T)> to_bytes(T value, Endianness endianness = Endianness::LittleEndian)
		{
			static_assert(std::is_trivially_copyable_v<T>, "to_bytes: T must be trivially copyable");
			static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "to_bytes: T must be 1, 2, 4, or 8 bytes");

			using U = detail::bit_container_t<T>;
			U u;
			std::memcpy(&u, &value, sizeof(T));   // bit-cast value -> unsigned

			std::array<i8, sizeof(T)> out{};
			if (endianness == Endianness::LittleEndian)
			{
				for (std::size_t i = 0; i < sizeof(T); ++i)
					out[i] = static_cast<i8>((u >> (i * 8)) & 0xFF);
			}
			else // BigEndian
			{
				for (std::size_t i = 0; i < sizeof(T); ++i)
					out[i] = static_cast<i8>((u >> ((sizeof(T) - 1 - i) * 8)) & 0xFF);
			}
			return out;
		}

		// Unpack bytes back into any 1/2/4/8-byte scalar.
		template<typename T>
		T from_bytes(std::span<const i8> bytes, Endianness endianness = Endianness::LittleEndian)
		{
			static_assert(std::is_trivially_copyable_v<T>, "from_bytes: T must be trivially copyable");
			static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "from_bytes: T must be 1, 2, 4, or 8 bytes");

			using U = detail::bit_container_t<T>;
			U u = 0;
			if (endianness == Endianness::LittleEndian)
			{
				for (std::size_t i = 0; i < sizeof(T); ++i)
					u |= static_cast<U>(static_cast<u8>(bytes[i])) << (i * 8);
			}
			else // BigEndian
			{
				for (std::size_t i = 0; i < sizeof(T); ++i)
					u |= static_cast<U>(static_cast<u8>(bytes[i])) << ((sizeof(T) - 1 - i) * 8);
			}

			T value;
			std::memcpy(&value, &u, sizeof(T));   // bit-cast unsigned -> T
			return value;
		}

		// Forward declaration
		class I_serializable;
	}
	class Serial
	{
		public: template<typename T> struct ReadResult { T value; bool error { false }; };
		public: using StreamEdit = std::span<int8_t>;
		public: using StreamView = std::span<const int8_t>;
		public: class Stream
		{
			public:
				Stream(const Stream&) = delete;
				Stream& operator=(const Stream&) = delete;
				inline Stream(void) {  } // Uninitialized Stream
				inline Stream(u64 size, serial::Endianness endianness = serial::Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0) {
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
				inline bool construct(u64 size, serial::Endianness endianness = serial::Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0) {
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
				inline bool isLittleEndian(void) const { return m_endianness == serial::Endianness::LittleEndian; }
				inline bool isBigEndian(void) const { return m_endianness == serial::Endianness::BigEndian; }
				inline serial::Endianness getEndianness(void) const { return m_endianness; }
				inline void setEndianness(serial::Endianness endianness) { m_endianness = endianness; }
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
				serial::Endianness m_endianness { serial::Endianness::LittleEndian };
		};
		public: struct StreamAccessor
		{
			public:
				bool resetAddress(u64 newAddr = 1);

			protected:
				inline StreamAccessor(Serial& _parent) : parent(_parent) {  }

				u64& resolve_address(u64& addr);
				bool restore_address_and_return(bool passthrough);
				void clear_address_backup(void);
				inline void disable_address_restore(void) { m_addressRestoreEnabled = false; }
				inline void enable_address_restore(void) { m_addressRestoreEnabled = true; }

				// Convenience wrappers
				inline bool isValid(void) const { return parent.isValid(); }
				inline bool isInvalid(void) const { return parent.isInvalid(); }
				inline bool isLittleEndian(void) const { return parent.isLittleEndian(); }
				inline bool isBigEndian(void) const { return parent.isBigEndian(); }
				inline bool isValidAddress(u64 addr, u32 size = 1) const { return parent.isValidAddress(addr, size); }

			protected:
				Serial& parent;
				StreamEdit data;
			private:
				u64 m_address { 1 };
				u64 m_backupAddress { 1 };
				bool m_addressBackupStored { false };
				bool m_addressRestoreEnabled { true };

				friend class Serial;
		};
		public: struct Reader : public StreamAccessor
		{
			inline Reader(Serial& _parent) : StreamAccessor(_parent) { }

			template<typename T>
			Serial::ReadResult<T> read_scalar(u64 _addr)
			{
				if (isInvalid()) return { T{}, true };
				u64& addr = resolve_address(_addr);
				if (!isValidAddress(addr, sizeof(T)))
					return { T{}, restore_address_and_return(true) };
				T val = from_bytes<T>(data.subspan(addr, sizeof(T)), parent.getEndianness());
				addr += sizeof(T);
				clear_address_backup();
				return { val, false };
			}

			inline ReadResult<i8> int8(u64 _addr)   { return read_scalar<i8>(_addr); }
			inline ReadResult<i8> int8(void) { return int8(0); }
			inline ReadResult<u8> uint8(u64 _addr)  { return read_scalar<u8>(_addr); }
			inline ReadResult<u8> uint8(void) { return uint8(0); }
			inline ReadResult<i16> int16(u64 _addr)  { return read_scalar<i16>(_addr); }
			inline ReadResult<i16> int16(void) { return int16(0); }
			inline ReadResult<u16> uint16(u64 _addr)  { return read_scalar<u16>(_addr); }
			inline ReadResult<u16> uint16(void) { return uint16(0); }
			inline ReadResult<i32> int32(u64 _addr)  { return read_scalar<i32>(_addr); }
			inline ReadResult<i32> int32(void) { return int32(0); }
			inline ReadResult<u32> uint32(u64 _addr)  { return read_scalar<u32>(_addr); }
			inline ReadResult<u32> uint32(void) { return uint32(0); }
			inline ReadResult<i64> int64(u64 _addr)  { return read_scalar<i64>(_addr); }
			inline ReadResult<i64> int64(void) { return int64(0); }
			inline ReadResult<u64> uint64(u64 _addr)  { return read_scalar<u64>(_addr); }
			inline ReadResult<u64> uint64(void) { return uint64(0); }
			inline ReadResult<f32> float32(u64 _addr) { return read_scalar<f32>(_addr); }
			inline ReadResult<f32> float32(void) { return float32(0); }
			inline ReadResult<f64> float64(u64 _addr) { return read_scalar<f64>(_addr); }
			inline ReadResult<f64> float64(void) { return float64(0); }

			bool stream(u64 _addr, Stream& outStream, bool sizeStored = true, u32 size_if_not_stored = 0);
			inline bool stream(Stream& outStream, bool sizeStored = true, u32 size_if_not_stored = 0) { return stream(0, outStream, sizeStored, size_if_not_stored); }
			bool string(u64 _addr, String& outString, bool sizeStored = true, u32 size_if_not_stored = 0, bool null_terminated = false);
			inline bool string(String& outString, bool sizeStored = true, u32 size_if_not_stored = 0, bool null_terminated = false) { return string(0, outString, sizeStored, size_if_not_stored, null_terminated); }
			bool object(u64 _addr, serial::I_serializable& outObject);
			inline bool object(serial::I_serializable& outObject) { return object(0, outObject); }
		};
		public: struct Writer : public StreamAccessor
		{
			inline Writer(Serial& _parent) : StreamAccessor(_parent) { }

			template<typename T>
			bool write_scalar(u64 _addr, T value)
			{
				if (isInvalid()) return false;
				u64& addr = resolve_address(_addr);
				if (!isValidAddress(addr, sizeof(T)))
					return restore_address_and_return(false);
				auto bytes = to_bytes(value, parent.getEndianness());
				std::copy(bytes.begin(), bytes.end(), data.begin() + addr);
				addr += sizeof(T);
				clear_address_backup();
				return true;
			}

			inline bool int8(u64 _addr, i8 value) { return write_scalar<i8> (_addr, value); }
			inline bool int8(i8 value) { return int8(0, value); }
			inline bool uint8(u64 _addr, u8 value) { return write_scalar<u8> (_addr, value); }
			inline bool uint8(u8 value) { return uint8(0, value); }
			inline bool int16(u64 _addr, i16 value) { return write_scalar<i16>(_addr, value); }
			inline bool int16(i16 value) { return int16(0, value); }
			inline bool uint16(u64 _addr, u16 value) { return write_scalar<u16>(_addr, value); }
			inline bool uint16(u16 value) { return uint16(0, value); }
			inline bool int32(u64 _addr, i32 value) { return write_scalar<i32>(_addr, value); }
			inline bool int32(i32 value) { return int32(0, value); }
			inline bool uint32(u64 _addr, u32 value) { return write_scalar<u32>(_addr, value); }
			inline bool uint32(u32 value) { return uint32(0, value); }
			inline bool int64(u64 _addr, i64 value) { return write_scalar<i64>(_addr, value); }
			inline bool int64(i64 value) { return int64(0, value); }
			inline bool uint64(u64 _addr, u64 value) { return write_scalar<u64>(_addr, value); }
			inline bool uint64(u64 value) { return uint64(0, value); }
			inline bool float32(u64 _addr, f32 value) { return write_scalar<f32>(_addr, value); }
			inline bool float32(f32 value) { return float32(0, value); }
			inline bool float64(u64 _addr, f64 value) { return write_scalar<f64>(_addr, value); }
			inline bool float64(f64 value) { return float64(0, value); }

			bool stream(u64 _addr, const Stream& value, bool storeSize = true);
			inline bool stream(const Stream& value, bool storeSize = true) { return stream(0, value, storeSize); }
			bool string(u64 _addr, const String& value, bool storeSize = true, bool null_terminated = false); // if sizeStored, null_terminated will be ignored
			inline bool string(const String& value, bool storeSize = true, bool null_terminated = false) { return string(0, value, storeSize, null_terminated); }
			bool object(u64 _addr, const serial::I_serializable& value);
			inline bool object(const serial::I_serializable& value) { return object(0, value); }
		};

		public:
			inline Serial(void) : read(*this), write(*this) { }
			inline Serial(u64 size, serial::Endianness endianness = serial::Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0) : read(*this), write(*this) { construct(size, endianness, preInitialize, initialValue); }
			bool construct(u64 size, serial::Endianness endianness = serial::Endianness::LittleEndian, bool preInitialize = false, i8 initialValue = 0);
			bool isValidAddress(u64 addr, u32 size = 1) const;

			inline bool isValid(void) const { return m_data.isValid(); }
			inline bool isInvalid(void) const { return !isValid(); }
			inline bool isLittleEndian(void) const { return m_data.isLittleEndian(); }
			inline bool isBigEndian(void) const { return m_data.isBigEndian(); }
			inline serial::Endianness getEndianness(void) const { return m_data.getEndianness(); }

		private:
			Stream m_data;

		public:
			Writer write;
			Reader read;
	};
	namespace serial
	{
		class I_serializable
		{
			public:
				virtual Serial::Stream serialize(void) const = 0;
				virtual bool deserialize(const Serial::Stream& data) = 0;
		};
	}
}
