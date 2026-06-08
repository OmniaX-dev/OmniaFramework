#include "Serial.hpp"
#include "Memory.hpp"

namespace ostd
{
	bool Serial::StreamAccessor::resetAddress(u64 newAddr)
	{
		if (newAddr == 0)
			newAddr = 1; // Managed Address should never be 0, if the intent is to reset the managed address to the start of the Stream it will be set to 1
		if (!isValidAddress(newAddr))
			return false;
		m_address = newAddr;
		m_addressBackupStored = false;
		m_backupAddress = newAddr;
		return true;
	}

	u64& Serial::StreamAccessor::resolve_address(u64& addr)
	{
		if (addr == 0)
		{
			m_backupAddress = m_address;
			m_addressBackupStored = true;
			return m_address;
		}
		return addr;
	}

	bool Serial::StreamAccessor::restore_address_and_return(bool passthrough)
	{
		if (!m_addressBackupStored || !m_addressRestoreEnabled) return passthrough;
		resetAddress(m_backupAddress);
		return passthrough;
	}

	void Serial::StreamAccessor::clear_address_backup(void)
	{
		if (!m_addressRestoreEnabled) return;
		m_addressBackupStored = false;
	}




	Serial::Serial(StreamView copyData, serial::Endianness endianness) : read(*this), write(*this)
	{
		if (construct(copyData.size(), endianness, false) && isValid())
		{
			std::ranges::copy(copyData, m_data.data().begin());
		}
	}

	bool Serial::construct(u64 size, serial::Endianness endianness, bool preInitialize, i8 initialValue)
	{
		if (m_data.isValid()) return false;
		if (!m_data.construct(size, endianness, preInitialize, initialValue))
			throw std::runtime_error("Serial::Stream::construct failed.");
		write.data = m_data.data();
		read.data = m_data.data();
		write.resetAddress();
		read.resetAddress();
		write.clear_address_backup();
		read.clear_address_backup();
		return true;
	}

	bool Serial::isValidAddress(u64 addr, u32 size) const
	{
		if (addr == 0) return false; // addr == 0 is managed mode and should never reach this point
		if (size == 0) return false;
		if (addr > u64Range::max() - size) return false;   // u64 overflow check
		return (addr + size - 1) < m_data.size();
	}

	bool Serial::writeToFile(const String& filePath) const
	{
		if (!isValid()) return false;
		return Memory::saveByteStreamToFile(m_data.data(), filePath);
	}




	bool Serial::Reader::stream(u64 _addr, Stream& outStream, bool sizeStored, u32 size_if_not_stored)
	{
		if (isInvalid()) return false;
		u64& addr = resolve_address(_addr);
		if (!sizeStored && size_if_not_stored < 1) return restore_address_and_return(false);
		u32 size = size_if_not_stored;
		if (sizeStored)
		{
			disable_address_restore();
			auto tmp = uint32(addr);
			enable_address_restore();
			if (tmp.error) return restore_address_and_return(false);
			addr += tTypeSize::UINT32;
			size = tmp.value;
		}
		serial::Endianness endianness;
		{
			disable_address_restore();
			auto tmp = uint8(addr);
			enable_address_restore();
			if (tmp.error) return restore_address_and_return(false);
			addr += tTypeSize::UINT8;
			if (tmp.value == cast<u8>(serial::Endianness::LittleEndian))
				endianness = serial::Endianness::LittleEndian;
			else if (tmp.value == cast<u8>(serial::Endianness::BigEndian))
				endianness = serial::Endianness::BigEndian;
			else return restore_address_and_return(false);
		}
		if (!isValidAddress(addr, size)) return restore_address_and_return(false);
		Serial::Stream _data = { size, endianness };
		StreamEdit edit = _data.data();
		std::copy_n(data.begin() + addr, size, edit.begin());
		outStream = std::move(_data);
		addr += size;
		clear_address_backup();
		return true;
	}

	bool Serial::Reader::string(u64 _addr, String& outString, bool sizeStored, u32 size_if_not_stored, bool null_terminated)
	{
		if (isInvalid()) return false;
		u64& addr = resolve_address(_addr);
		if (!null_terminated || sizeStored) // I want null_terminated to have the lowest priority
		{
			if (!sizeStored && size_if_not_stored < 1) return restore_address_and_return(false);
			u32 size = size_if_not_stored;
			if (sizeStored)
			{
				disable_address_restore();
				auto tmp = uint32(addr);
				enable_address_restore();
				if (tmp.error) return restore_address_and_return(false);
				addr += tTypeSize::UINT32;
				size = tmp.value;
			}
			if (size > 0 && !isValidAddress(addr, size)) return restore_address_and_return(false);
			outString = "";
			for (u64 i = addr; i < (addr + size); i++)
			{
				outString += cast<char>(data[i]);
			}
			addr += size;
			clear_address_backup();
			return true;
		}
		else
		{
			String out = "";
			disable_address_restore();
			auto tmp = int8(addr);
			enable_address_restore();
			if (tmp.error) return restore_address_and_return(false);
			addr += tTypeSize::INT8;
			while (tmp.value != '\0')
			{
				out += cast<char>(tmp.value);
				disable_address_restore();
				tmp = int8(addr);
				enable_address_restore();
				if (tmp.error) return restore_address_and_return(false);
				addr += tTypeSize::INT8;
			}
			outString = out;
			clear_address_backup();
			return true;
		}
	}

	bool Serial::Reader::object(u64 _addr, serial::I_serializable& outObject)
	{
		Serial::Stream objData;
		if (!stream(_addr, objData, true, 0))
			return false;
		return outObject.deserialize(objData);
	}




	bool Serial::Writer::stream(u64 _addr, const Stream& value, bool storeSize)
	{
		if (isInvalid()) return false;
		u64& addr = resolve_address(_addr);
		u32 size = value.size();
		if (size < 1) return restore_address_and_return(false);
		if (storeSize)
		{
			disable_address_restore();
			bool res = uint32(addr, size);
			enable_address_restore();
			if (!res) return restore_address_and_return(false);
			addr += tTypeSize::UINT32;
		}
		disable_address_restore();
		bool res = uint8(addr, cast<u8>(value.getEndianness()));
		enable_address_restore();
		if (!res)
			return restore_address_and_return(false);
		addr += tTypeSize::UINT8;
		if (!isValidAddress(addr, size)) return restore_address_and_return(false);
		StreamView view = value.data();
		std::copy_n(view.begin(), size, data.begin() + addr);
		addr += size;
		clear_address_backup();
		return true;
	}

	bool Serial::Writer::string(u64 _addr, const String& value, bool storeSize, bool null_terminated)
	{
		if (isInvalid()) return false;
		u64& addr = resolve_address(_addr);
		u32 size = value.len();
		if (storeSize)
		{
			null_terminated = false;
			disable_address_restore();
			bool res = uint32(addr, size);
			enable_address_restore();
			if (!res) return restore_address_and_return(false);
			addr += tTypeSize::UINT32;
		}
		size += (null_terminated ? 1 : 0);
		if (size > 0 && !isValidAddress(addr, size))
			return restore_address_and_return(false);
		for (u64 i = 0; i < value.len(); i++)
			data[addr + i] = cast<i8>(value[i]);
		if (null_terminated)
			data[addr + value.len()] = '\0';
		addr += size;
		clear_address_backup();
		return true;
	}

	bool Serial::Writer::object(u64 _addr, const serial::I_serializable& value)
	{
		if (isInvalid()) return false; // Even though return stream(...) will check this again, this is here to avoid serializing for nothing, since it might be an expensive operation
		const Serial::Stream data = value.serialize();
		if (data.size() < 1) return false;
		return stream(_addr, data, true);
	}
}
