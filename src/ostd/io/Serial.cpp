#include "Serial.hpp"
#include "io/Memory.hpp"

namespace ostd
{
	bool Serial::isValidAddress(u64 addr, u32 size)
	{
		if (addr == 0) return false;
		if (size == 0) return false;
		if (addr > u64Range::max() - size) return false;   // u64 overflow check
		return (addr + size - 1) < s_openStream.size();
	}

	Serial::ReadResult<i8> Serial::read::int8(u64 addr)
	{
		if (!isStreamAttached()) return { 0, true };
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT8)) return { 0, true };
		i8 val = s_openStream[addr];
		s_addr += tTypeSize::INT8;
		return { val, false };
	}

	Serial::ReadResult<u8> Serial::read::uint8(u64 addr)
	{
		auto res = int8(addr);
		return { cast<u8>(res.value), res.error };
	}

	Serial::ReadResult<i16> Serial::read::int16(u64 addr)
	{
		if (!isStreamAttached()) return { 0, true };
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT16)) return { 0, true };
		i16 val = 0;
		if (isLittleEndian())
		{
			val = ((s_openStream[addr + 0]     ) & 0x00FFU)
				| ((s_openStream[addr + 1] << 8) & 0xFF00U);
		}
		else if (isBigEndian())
		{
			val = ((s_openStream[addr + 0] <<  8) & 0xFF00U)
				| ( s_openStream[addr + 1]        & 0x00FFU);
		}
		else return { 0, true };
		s_addr += tTypeSize::INT16;
		return { val, false };
	}

	Serial::ReadResult<u16> Serial::read::uint16(u64 addr)
	{
		auto res = int16(addr);
		return { cast<u16>(res.value), res.error };
	}

	Serial::ReadResult<i32> Serial::read::int32(u64 addr)
	{
		if (!isStreamAttached()) return { 0, true };
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT32)) return { 0, true };
		i32 val = 0;
		if (isLittleEndian())
		{
			val = ((s_openStream[addr + 0]      ) & 0x000000FF)
				| ((s_openStream[addr + 1] <<  8) & 0x0000FF00)
				| ((s_openStream[addr + 2] << 16) & 0x00FF0000)
				| ((s_openStream[addr + 3] << 24) & 0xFF000000);
		}
		else if (isBigEndian())
		{
			val = ((s_openStream[addr + 0] << 24) & 0xFF000000U)
				| ((s_openStream[addr + 1] << 16) & 0x00FF0000U)
				| ((s_openStream[addr + 2] <<  8) & 0x0000FF00U)
				| ( s_openStream[addr + 3]        & 0x000000FFU);
		}
		else return { 0, true };
		s_addr += tTypeSize::INT32;
		return { val, false };
	}

	Serial::ReadResult<u32> Serial::read::uint32(u64 addr)
	{
		auto res = int32(addr);
		return { cast<u32>(res.value), res.error };
	}

	Serial::ReadResult<i64> Serial::read::int64(u64 addr)
	{
		if (!isStreamAttached()) return { 0, true };
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT64)) return { 0, true };
		i64 val = 0;
		if (isLittleEndian())
		{
			val = (      s_openStream[addr + 7]        & 0x00000000000000FFU)
				| ((     s_openStream[addr + 6] <<  8) & 0x000000000000FF00U)
				| ((     s_openStream[addr + 5] << 16) & 0x0000000000FF0000U)
				| ((     s_openStream[addr + 4] << 24) & 0x00000000FF000000U)
				| (((i64)s_openStream[addr + 3] << 32) & 0x000000FF00000000U)
				| (((i64)s_openStream[addr + 2] << 40) & 0x0000FF0000000000U)
				| (((i64)s_openStream[addr + 1] << 48) & 0x00FF000000000000U)
				| (((i64)s_openStream[addr + 0] << 56) & 0xFF00000000000000U);
		}
		else if (isBigEndian())
		{
			val = (((i64)s_openStream[addr + 0] << 56) & 0xFF00000000000000U)
				| (((i64)s_openStream[addr + 1] << 48) & 0x00FF000000000000U)
				| (((i64)s_openStream[addr + 2] << 40) & 0x0000FF0000000000U)
				| (((i64)s_openStream[addr + 3] << 32) & 0x000000FF00000000U)
				| ((     s_openStream[addr + 4] << 24) & 0x00000000FF000000U)
				| ((     s_openStream[addr + 5] << 16) & 0x0000000000FF0000U)
				| ((     s_openStream[addr + 6] <<  8) & 0x000000000000FF00U)
				| (      s_openStream[addr + 7]        & 0x00000000000000FFU);
		}
		else return { 0, true };
		s_addr += tTypeSize::INT64;
		return { val, false };
	}

	Serial::ReadResult<u64> Serial::read::uint64(u64 addr)
	{
		auto res = int64(addr);
		return { cast<u64>(res.value), res.error };
	}

	Serial::ReadResult<f32> Serial::read::float32(u64 addr)
	{
		if (!isStreamAttached()) return { 0, true };
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::FLOAT32)) return { 0, true };
		__float_parser fp;
		if (isLittleEndian())
		{
			fp.data[3] = s_openStream[addr + 0];
			fp.data[2] = s_openStream[addr + 1];
			fp.data[1] = s_openStream[addr + 2];
			fp.data[0] = s_openStream[addr + 3];
		}
		else if (isBigEndian())
		{
			fp.data[0] = s_openStream[addr + 0];
			fp.data[1] = s_openStream[addr + 1];
			fp.data[2] = s_openStream[addr + 2];
			fp.data[3] = s_openStream[addr + 3];
		}
		else return { 0, true };
		s_addr += tTypeSize::FLOAT32;
		return { fp.val, false };
	}

	Serial::ReadResult<f64> Serial::read::float64(u64 addr)
	{
		if (!isStreamAttached()) return { 0, true };
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::FLOAT64)) return { 0, true };
		__double_parser dp;
		if (isLittleEndian())
		{
			dp.data[7] = s_openStream[addr++];
			dp.data[6] = s_openStream[addr++];
			dp.data[5] = s_openStream[addr++];
			dp.data[4] = s_openStream[addr++];
			dp.data[3] = s_openStream[addr++];
			dp.data[2] = s_openStream[addr++];
			dp.data[1] = s_openStream[addr++];
			dp.data[0] = s_openStream[addr  ];
		}
		else if (isBigEndian())
		{
			dp.data[0] = s_openStream[addr++];
			dp.data[1] = s_openStream[addr++];
			dp.data[2] = s_openStream[addr++];
			dp.data[3] = s_openStream[addr++];
			dp.data[4] = s_openStream[addr++];
			dp.data[5] = s_openStream[addr++];
			dp.data[6] = s_openStream[addr++];
			dp.data[7] = s_openStream[addr  ];
		}
		else return { 0, true };
		s_addr += tTypeSize::FLOAT64;
		return { dp.val, false };
	}

	bool Serial::read::stream(Stream& outStream, u64 addr, bool sizeStored, u32 size_if_not_stored)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!sizeStored && size_if_not_stored < 1) return false;
		u32 size = size_if_not_stored;
		if (sizeStored)
		{
			auto tmp = uint32(addr);
			if (tmp.error) return false;
			addr += tTypeSize::UINT32;
			size = tmp.value;
		}
		Endianness endianness;
		{
			auto tmp = uint8(addr);
			if (tmp.error) return false;
			addr += tTypeSize::UINT8;
			if (tmp.value == cast<u8>(Endianness::LittleEndian))
				endianness = Endianness::LittleEndian;
			else if (tmp.value == cast<u8>(Endianness::BigEndian))
				endianness = Endianness::BigEndian;
			else return false;
		}
		if (!isValidAddress(addr, size)) return false;
		Serial::Stream data = { size, endianness };
		StreamEdit edit = data.data();
		std::copy_n(s_openStream.begin() + addr, size, edit.begin());
		outStream = std::move(data);
		s_addr = addr + size;
		return true;
	}

	bool Serial::read::string(String& outString, u64 addr, bool sizeStored, u32 size_if_not_stored, bool null_terminated)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!null_terminated || sizeStored) // I want null_terminated to have the lowest priority
		{
			if (!sizeStored && size_if_not_stored < 1) return false;
			u32 size = size_if_not_stored;
			if (sizeStored)
			{
				auto tmp = uint32(addr);
				if (tmp.error) return false;
				addr += tTypeSize::UINT32;
				size = tmp.value;
			}
			if (!isValidAddress(addr, size)) return false;
			for (u64 i = addr; i < (addr + size); i++)
			{
				outString += cast<char>(s_openStream[i]);
			}
			s_addr = addr + size;
			return true;
		}
		else
		{
			String out = "";
			auto tmp = int8(addr);
			if (tmp.error) return false;
			addr += tTypeSize::INT8;
			while (tmp.value != '\0')
			{
				out += cast<char>(tmp.value);
				tmp = int8(addr);
				if (tmp.error) return false;
				addr += tTypeSize::INT8;
			}
			outString = out;
			s_addr = addr;
			return true;
		}
	}

	bool Serial::read::object(__i_serializable& outObject, u64 addr)
	{
		Serial::Stream objData(0);
		if (!stream(objData, addr, true, 0))
			return false;
		return outObject.deserialize(objData);
	}



	bool Serial::write::int8(u64 addr, i8 value)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT8)) return false;
		s_openStream[addr] = value;
		s_addr += tTypeSize::INT8;
		return true;
	}

	bool Serial::write::uint8(u64 addr, u8 value)
	{
		return int8(addr, cast<i8>(value));
	}

	bool Serial::write::int16(u64 addr, i16 value)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT16)) return false;
		if (isLittleEndian())
		{
			s_openStream[addr + 0] = value & 0xFF;
			s_openStream[addr + 1] = (value >> 8) & 0xFF;
		}
		else if (isBigEndian())
		{
			s_openStream[addr + 0] = (value >> 8) & 0xFF;
			s_openStream[addr + 1] = value & 0xFF;
		}
		else return false;
		s_addr += tTypeSize::INT16;
		return true;
	}

	bool Serial::write::uint16(u64 addr, u16 value)
	{
		return int16(addr, cast<i16>(value));
	}

	bool Serial::write::int32(u64 addr, i32 value)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT32)) return false;
		if (isLittleEndian())
		{
			s_openStream[addr + 0] = value & 0xFF;
			s_openStream[addr + 1] = (value >> 8) & 0xFF;
			s_openStream[addr + 2] = (value >> 16) & 0xFF;
			s_openStream[addr + 3] = (value >> 24) & 0xFF;
		}
		else if (isBigEndian())
		{
			s_openStream[addr + 0] = (value >> 24) & 0xFF;
			s_openStream[addr + 1] = (value >> 16) & 0xFF;
			s_openStream[addr + 2] = (value >> 8) & 0xFF;
			s_openStream[addr + 3] = value & 0xFF;
		}
		else return false;
		s_addr += tTypeSize::INT32;
		return true;
	}

	bool Serial::write::uint32(u64 addr, u32 value)
	{
		return int32(addr, cast<i32>(value));
	}

	bool Serial::write::int64(u64 addr, i64 value)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::INT64)) return false;
		if (isLittleEndian())
		{
			s_openStream[addr + 0] = value & 0xFF;
			s_openStream[addr + 1] = (value >> 8) & 0xFF;
			s_openStream[addr + 2] = (value >> 16) & 0xFF;
			s_openStream[addr + 3] = (value >> 24) & 0xFF;
			s_openStream[addr + 4] = (value >> 32) & 0xFF;
			s_openStream[addr + 5] = (value >> 40) & 0xFF;
			s_openStream[addr + 6] = (value >> 48) & 0xFF;
			s_openStream[addr + 7] = (value >> 56) & 0xFF;
		}
		else if (isBigEndian())
		{
			s_openStream[addr + 0] = (value >> 56) & 0xFF;
			s_openStream[addr + 1] = (value >> 48) & 0xFF;
			s_openStream[addr + 2] = (value >> 40) & 0xFF;
			s_openStream[addr + 3] = (value >> 32) & 0xFF;
			s_openStream[addr + 4] = (value >> 24) & 0xFF;
			s_openStream[addr + 5] = (value >> 16) & 0xFF;
			s_openStream[addr + 6] = (value >> 8) & 0xFF;
			s_openStream[addr + 7] = value & 0xFF;
		}
		else return false;
		s_addr += tTypeSize::INT64;
		return true;
	}

	bool Serial::write::uint64(u64 addr, u64 value)
	{
		if (!isStreamAttached()) return false;
		if (!isValidAddress(addr, tTypeSize::UINT64)) return false;
		return int64(addr, cast<i64>(value));
	}

	bool Serial::write::float32(u64 addr, f32 value)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::FLOAT32)) return false;
		__float_parser fp;
		fp.val = value;
		if (isLittleEndian())
		{
			s_openStream[addr + 0] = fp.data[3];
			s_openStream[addr + 1] = fp.data[2];
			s_openStream[addr + 2] = fp.data[1];
			s_openStream[addr + 3] = fp.data[0];
		}
		else if (isBigEndian())
		{
			s_openStream[addr + 0] = fp.data[0];
			s_openStream[addr + 1] = fp.data[1];
			s_openStream[addr + 2] = fp.data[2];
			s_openStream[addr + 3] = fp.data[3];
		}
		else return false;
		s_addr += tTypeSize::FLOAT32;
		return true;
	}

	bool Serial::write::float64(u64 addr, f64 value)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		if (!isValidAddress(addr, tTypeSize::FLOAT64)) return false;
		__double_parser dp;
		dp.val = value;
		if (isLittleEndian())
		{
			s_openStream[addr++] = dp.data[7];
			s_openStream[addr++] = dp.data[6];
			s_openStream[addr++] = dp.data[5];
			s_openStream[addr++] = dp.data[4];
			s_openStream[addr++] = dp.data[3];
			s_openStream[addr++] = dp.data[2];
			s_openStream[addr++] = dp.data[1];
			s_openStream[addr  ] = dp.data[0];
		}
		else if (isBigEndian())
		{
			s_openStream[addr++] = dp.data[0];
			s_openStream[addr++] = dp.data[1];
			s_openStream[addr++] = dp.data[2];
			s_openStream[addr++] = dp.data[3];
			s_openStream[addr++] = dp.data[4];
			s_openStream[addr++] = dp.data[5];
			s_openStream[addr++] = dp.data[6];
			s_openStream[addr  ] = dp.data[7];
		}
		else return false;
		s_addr += tTypeSize::FLOAT64;
		return true;
	}

	bool Serial::write::stream(u64 addr, const Stream& value, bool storeSize)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		u32 size = value.size();
		if (size < 1) return false;
		if (storeSize)
		{
			if (!uint32(addr, size)) return false;
			addr += tTypeSize::UINT32;
		}
		if (!isValidAddress(addr, size)) return false;
		StreamView view = value.data();
		for (u64 i = addr; i < addr + size; i++)
			s_openStream[i] = view[i - addr];
		s_addr = addr + size;
		return true;
	}

	bool Serial::write::string(u64 addr, const String& value, bool storeSize, bool null_terminated)
	{
		if (!isStreamAttached()) return false;
		managed_address_check(addr);
		u32 size = value.len();
		if (size < 1) return false;
		if (storeSize)
		{
			null_terminated = false;
			if (!uint32(addr, value.len())) return false;
			addr += tTypeSize::UINT32;
		}
		size += (null_terminated ? 1 : 0);
		if (!isValidAddress(addr, size))
			return false;
		for (u64 i = addr; i < addr + size; i++)
		{
			i8 c = cast<i8>(value[i - addr]);
			if (null_terminated && i == addr + size - 1)
				c = '\0';
			s_openStream[i] = c;
		}
		s_addr = addr + size;
		return true;
	}

	bool Serial::write::object(u64 addr, const __i_serializable& value)
	{
		if (!isStreamAttached()) return false;
		const Serial::Stream data = value.serialize();
		if (data.size() < 1) return false;
		return stream(addr, data, true);
	}





	// =============================================== OLD VERSION ===============================================
	namespace serial
	{
		bool SerialIO::init(u64 size, u8 endianness)
		{
			if (size < 1) return false;
			for (u64 i = 0; i < size; i++)
				m_data.push_back(0);
			m_endianness = endianness;
			return true;
		}


		bool SerialIO::r_QWord(StreamIndex addr, QWord& outVal)
		{
			if (!is_validAddr(addr, tTypeSize::QWORD)) return false;
			if (isLittleEndian())
			{
				outVal =  (        m_data[addr + 7]        & 0x00000000000000FFU)
						| ((       m_data[addr + 6] <<  8) & 0x000000000000FF00U)
						| ((       m_data[addr + 5] << 16) & 0x0000000000FF0000U)
						| ((       m_data[addr + 4] << 24) & 0x00000000FF000000U)
						| (((QWord)m_data[addr + 3] << 32) & 0x000000FF00000000U)
						| (((QWord)m_data[addr + 2] << 40) & 0x0000FF0000000000U)
						| (((QWord)m_data[addr + 1] << 48) & 0x00FF000000000000U)
						| (((QWord)m_data[addr + 0] << 56) & 0xFF00000000000000U);
			}
			else if (isBigEndian())
			{
				outVal =  (((QWord)m_data[addr + 0] << 56) & 0xFF00000000000000U)
						| (((QWord)m_data[addr + 1] << 48) & 0x00FF000000000000U)
						| (((QWord)m_data[addr + 2] << 40) & 0x0000FF0000000000U)
						| (((QWord)m_data[addr + 3] << 32) & 0x000000FF00000000U)
						| ((       m_data[addr + 4] << 24) & 0x00000000FF000000U)
						| ((       m_data[addr + 5] << 16) & 0x0000000000FF0000U)
						| ((       m_data[addr + 6] <<  8) & 0x000000000000FF00U)
						| (        m_data[addr + 7]        & 0x00000000000000FFU);
			}
			else return false;
			return true;
		}

		bool SerialIO::r_DWord(StreamIndex addr, DWord& outVal)
		{
			if (!is_validAddr(addr, tTypeSize::DWORD)) return false;
			if (isLittleEndian())
			{
				outVal =  ((m_data[addr + 0]      ) & 0x000000FF)
						| ((m_data[addr + 1] <<  8) & 0x0000FF00)
						| ((m_data[addr + 2] << 16) & 0x00FF0000)
						| ((m_data[addr + 3] << 24) & 0xFF000000);
			}
			else if (isBigEndian())
			{
				outVal =  ((m_data[addr + 0] << 24) & 0xFF000000U)
						| ((m_data[addr + 1] << 16) & 0x00FF0000U)
						| ((m_data[addr + 2] <<  8) & 0x0000FF00U)
						| ( m_data[addr + 3]        & 0x000000FFU);
			}
			else return false;
			return true;
		}

		bool SerialIO::r_Word(StreamIndex addr, Word& outVal)
		{
			if (!is_validAddr(addr, tTypeSize::WORD)) return false;
			if (isLittleEndian())
			{
				outVal =  ((m_data[addr + 0]     ) & 0x00FFU)
						| ((m_data[addr + 1] << 8) & 0xFF00U);
			}
			else if (isBigEndian())
			{
				outVal =  ((m_data[addr + 0] <<  8) & 0xFF00U)
						| ( m_data[addr + 1]        & 0x00FFU);
			}
			else return false;
			return true;
		}

		bool SerialIO::r_Byte(StreamIndex addr, Byte& outVal)
		{
			if (!is_validAddr(addr, tTypeSize::BYTE)) return false;
			outVal = m_data[addr];
			return true;
		}

		bool SerialIO::r_Addr(StreamIndex addr, StreamIndex& outVal)
		{
			if (!is_validAddr(addr, tTypeSize::ADDR)) return false;
			if (isLittleEndian())
			{
				outVal =  ((m_data[addr + 0]      ) & 0x000000FF)
						| ((m_data[addr + 1] <<  8) & 0x0000FF00)
						| ((m_data[addr + 2] << 16) & 0x00FF0000)
						| ((m_data[addr + 3] << 24) & 0xFF000000);
			}
			else if (isBigEndian())
			{
				outVal =  ((m_data[addr + 0] << 24) & 0xFF000000U)
						| ((m_data[addr + 1] << 16) & 0x00FF0000U)
						| ((m_data[addr + 2] <<  8) & 0x0000FF00U)
						| ( m_data[addr + 3]        & 0x000000FFU);
			}
			else return false;
			return true;
		}

		bool SerialIO::r_Float(StreamIndex addr, f32& outVal)
		{
			if (!is_validAddr(addr, tTypeSize::FLOAT)) return false;
			__float_parser fp;
			if (isLittleEndian())
			{
				fp.data[3] = m_data[addr + 0];
				fp.data[2] = m_data[addr + 1];
				fp.data[1] = m_data[addr + 2];
				fp.data[0] = m_data[addr + 3];
			}
			else if (isBigEndian())
			{
				fp.data[0] = m_data[addr + 0];
				fp.data[1] = m_data[addr + 1];
				fp.data[2] = m_data[addr + 2];
				fp.data[3] = m_data[addr + 3];
			}
			else return false;
			outVal = fp.val;
			return true;
		}

		bool SerialIO::r_Double(StreamIndex addr, f64& outVal)
		{
			if (!is_validAddr(addr, tTypeSize::DOUBLE)) return false;
			__double_parser dp;
			if (isLittleEndian())
			{
				dp.data[7] = m_data[addr++];
				dp.data[6] = m_data[addr++];
				dp.data[5] = m_data[addr++];
				dp.data[4] = m_data[addr++];
				dp.data[3] = m_data[addr++];
				dp.data[2] = m_data[addr++];
				dp.data[1] = m_data[addr++];
				dp.data[0] = m_data[addr  ];
			}
			else if (isBigEndian())
			{
				dp.data[0] = m_data[addr++];
				dp.data[1] = m_data[addr++];
				dp.data[2] = m_data[addr++];
				dp.data[3] = m_data[addr++];
				dp.data[4] = m_data[addr++];
				dp.data[5] = m_data[addr++];
				dp.data[6] = m_data[addr++];
				dp.data[7] = m_data[addr  ];
			}
			else return false;
			outVal = dp.val;
			return true;
		}

		bool SerialIO::r_ByteStream(StreamIndex addr, ByteStream& outStream)
		{
			if (!is_validAddr(addr, tTypeSize::ADDR)) return false;
			u32 stream_size;
			if (!r_Addr(addr, stream_size)) return false;
			addr += tTypeSize::ADDR;
			if (!is_validAddr(addr, stream_size)) return false;
			outStream.clear();
			outStream.reserve(stream_size);
			for (StreamIndex j = addr; j < addr + stream_size; j++)
				outStream.push_back(m_data[j]);
			return true;
		}

		bool SerialIO::r_ByteStream(StreamIndex addr, ByteStream& outStream, u32 size)
		{
			if (!is_validAddr(addr, size)) return false;
			outStream.clear();
			outStream.reserve(size);
			for (StreamIndex j = addr; j < addr + size; j++)
				outStream.push_back(m_data[j]);
			return true;
		}

		bool SerialIO::r_String(StreamIndex addr, String& outString)
		{
			if (!is_validAddr(addr, tTypeSize::ADDR)) return false;
			u32 stream_size;
			if (!r_Addr(addr, stream_size)) return false;
			addr += tTypeSize::ADDR;
			if (!is_validAddr(addr, stream_size)) return false;
			outString = "";
			for (StreamIndex j = addr; j < addr + stream_size; j++)
				outString += (char)(m_data[j]);
			return true;
		}

		bool SerialIO::r_String(StreamIndex addr, String& outString, u32 size)
		{
			if (!is_validAddr(addr, size)) return false;
			outString = "";
			for (StreamIndex j = addr; j < addr + size; j++)
				outString += (char)(m_data[j]);
			return true;
		}

		bool SerialIO::r_NullTerminatedString(StreamIndex addr, String& outString)
		{
			outString = "";
			i8 c = 0;
			if (!r_Byte(addr, c)) return false;
			addr += tTypeSize::BYTE;
			while (c != 0)
			{
				outString += (char)c;
				if (!r_Byte(addr, c)) return false;
				addr += tTypeSize::BYTE;
			}
			return true;
		}


		bool SerialIO::w_QWord(StreamIndex addr, QWord val)
		{
			m_statuWriting = true;
			if (!is_validAddr(addr, tTypeSize::QWORD)) return false;
			if (isLittleEndian())
			{
				m_data[addr + 0] = val & 0xFF;
				m_data[addr + 1] = (val >> 8) & 0xFF;
				m_data[addr + 2] = (val >> 16) & 0xFF;
				m_data[addr + 3] = (val >> 24) & 0xFF;
				m_data[addr + 4] = (val >> 32) & 0xFF;
				m_data[addr + 5] = (val >> 40) & 0xFF;
				m_data[addr + 6] = (val >> 48) & 0xFF;
				m_data[addr + 7] = (val >> 56) & 0xFF;
			}
			else if (isBigEndian())
			{
				m_data[addr + 0] = (val >> 56) & 0xFF;
				m_data[addr + 1] = (val >> 48) & 0xFF;
				m_data[addr + 2] = (val >> 40) & 0xFF;
				m_data[addr + 3] = (val >> 32) & 0xFF;
				m_data[addr + 4] = (val >> 24) & 0xFF;
				m_data[addr + 5] = (val >> 16) & 0xFF;
				m_data[addr + 6] = (val >> 8) & 0xFF;
				m_data[addr + 7] = val & 0xFF;
			}
			else return false;
			return true;
		}

		bool SerialIO::w_DWord(StreamIndex addr, DWord val)
		{
			m_statuWriting = true;
			if (!is_validAddr(addr, tTypeSize::DWORD)) return false;
			if (isLittleEndian())
			{
				m_data[addr + 0] = val & 0xFF;
				m_data[addr + 1] = (val >> 8) & 0xFF;
				m_data[addr + 2] = (val >> 16) & 0xFF;
				m_data[addr + 3] = (val >> 24) & 0xFF;
			}
			else if (isBigEndian())
			{
				m_data[addr + 0] = (val >> 24) & 0xFF;
				m_data[addr + 1] = (val >> 16) & 0xFF;
				m_data[addr + 2] = (val >> 8) & 0xFF;
				m_data[addr + 3] = val & 0xFF;
			}
			else return false;
			return true;
		}

		bool SerialIO::w_Word(StreamIndex addr, Word val)
		{
			m_statuWriting = true;
			if (!is_validAddr(addr, tTypeSize::WORD)) return false;
			if (isLittleEndian())
			{
				m_data[addr + 0] = val & 0xFF;
				m_data[addr + 1] = (val >> 8) & 0xFF;
			}
			else if (isBigEndian())
			{
				m_data[addr + 0] = (val >> 8) & 0xFF;
				m_data[addr + 1] = val & 0xFF;
			}
			else return false;
			return true;
		}

		bool SerialIO::w_Byte(StreamIndex addr, Byte val)
		{
			m_statuWriting = true;
			if (!is_validAddr(addr, tTypeSize::BYTE)) return false;
			m_data[addr] = val;
			return true;
		}

		bool SerialIO::w_Addr(StreamIndex addr, StreamIndex val)
		{
			m_statuWriting = true;
			if (!is_validAddr(addr, tTypeSize::ADDR)) return false;
			if (isLittleEndian())
			{
				m_data[addr + 0] = val & 0xFF;
				m_data[addr + 1] = (val >> 8) & 0xFF;
				m_data[addr + 2] = (val >> 16) & 0xFF;
				m_data[addr + 3] = (val >> 24) & 0xFF;
			}
			else if (isBigEndian())
			{
				m_data[addr + 0] = (val >> 24) & 0xFF;
				m_data[addr + 1] = (val >> 16) & 0xFF;
				m_data[addr + 2] = (val >> 8) & 0xFF;
				m_data[addr + 3] = val & 0xFF;
			}
			else return false;
			return true;
		}

		bool SerialIO::w_Float(StreamIndex addr, f32 val)
		{
			m_statuWriting = true;
			if (!is_validAddr(addr, tTypeSize::FLOAT)) return false;
			__float_parser fp;
			fp.val = val;
			if (isLittleEndian())
			{
				m_data[addr + 0] = fp.data[3];
				m_data[addr + 1] = fp.data[2];
				m_data[addr + 2] = fp.data[1];
				m_data[addr + 3] = fp.data[0];
			}
			else if (isBigEndian())
			{
				m_data[addr + 0] = fp.data[0];
				m_data[addr + 1] = fp.data[1];
				m_data[addr + 2] = fp.data[2];
				m_data[addr + 3] = fp.data[3];
			}
			else return false;
			return true;
		}

		bool SerialIO::w_Double(StreamIndex addr, f64 val)
		{
			m_statuWriting = true;
			if (!is_validAddr(addr, tTypeSize::DOUBLE)) return false;
			__double_parser dp;
			dp.val = val;
			if (isLittleEndian())
			{
				m_data[addr++] = dp.data[7];
				m_data[addr++] = dp.data[6];
				m_data[addr++] = dp.data[5];
				m_data[addr++] = dp.data[4];
				m_data[addr++] = dp.data[3];
				m_data[addr++] = dp.data[2];
				m_data[addr++] = dp.data[1];
				m_data[addr  ] = dp.data[0];
			}
			else if (isBigEndian())
			{
				m_data[addr++] = dp.data[0];
				m_data[addr++] = dp.data[1];
				m_data[addr++] = dp.data[2];
				m_data[addr++] = dp.data[3];
				m_data[addr++] = dp.data[4];
				m_data[addr++] = dp.data[5];
				m_data[addr++] = dp.data[6];
				m_data[addr  ] = dp.data[7];
			}
			else return false;
			return true;
		}

		bool SerialIO::w_ByteStream(StreamIndex addr, const ByteStream& stream, bool store_size)
		{
			m_statuWriting = true;
			u32 stream_size = stream.size();
			if (store_size)
			{
				if (!is_validAddr(addr, tTypeSize::ADDR + stream_size)) return false;
				if (!w_Addr(addr, stream_size)) return false;
				addr += tTypeSize::ADDR;
			}
			else if (!is_validAddr(addr, stream_size)) return false;
			for (StreamIndex j = addr; j < addr + stream_size; j++)
				m_data[j] = stream[j - addr];
			return true;
		}

		bool SerialIO::w_String(StreamIndex addr, const String& str, bool store_size, bool null_terminate)
		{
			m_statuWriting = true;
			auto stream = Memory::stringToByteStream(str);
			u32 stream_size = stream.size();
			if (store_size && !null_terminate)
			{
				if (!is_validAddr(addr, tTypeSize::ADDR + stream_size)) return false;
				if (!w_Addr(addr, stream_size)) return false;
				addr += tTypeSize::ADDR;
			}
			else if (!is_validAddr(addr, stream_size)) return false;
			for (StreamIndex j = addr; j < addr + stream_size; j++)
				m_data[j] = stream[j - addr];
			addr += str.len() * tTypeSize::BYTE;
			if (null_terminate)
				if (!w_Byte(addr, 0x00)) return false;
			return true;
		}


		bool SerialIO::is_validAddr(StreamIndex addr, StreamIndex offset)
		{
			if ((addr + offset - 1) >= m_data.size())
			{
				if (isAutoResizeEnabled() && m_statuWriting)
				{
					for (u32 i = 0; i <= m_resizeAmount; i++)
						m_data.push_back(0);
					m_statuWriting = false;
					return true;
				}
				else
				{
					m_statuWriting = false;
					return false;
				}
			}
			m_statuWriting = false;
			return true;
		}

		void SerialIO::print(StreamIndex start, OutputHandlerBase& out)
		{
			u32 line_len = 32;
			u64 power = 1;
			while(power < size())
				power *= 2;
			Memory::printByteStream(m_data, start, line_len, power / line_len, out);
		}

		bool SerialIO::saveToFile(const String& filePath)
		{
			return Memory::saveByteStreamToFile(getData(), filePath);
		}
	}
}
