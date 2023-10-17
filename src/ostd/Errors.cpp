#include "Errors.hpp"
#include "Logger.hpp"

namespace ostd
{
	RuntimeError& RuntimeError::create(uint8_t group, uint64_t code, uint8_t level, const String &msg)
	{
		m_errGroup = group;
		m_errCode = code;
		m_message = msg;
		m_errLevel = level;
		setTypeName("ostd::RuntimeError");
		validate();
		return *this;
	}

	void RuntimeError::fire(const String& extraMessage, IOutputHandler* outputHandler, BaseObject& userData, int32_t _line_num, const String& _file_name)
	{
		if (isInvalid() || m_errGroup == 0x0 || m_errLevel == tErrorLevel::NoError || m_errCode == 0x0) return;
		String errorMessage = m_message + "\n";
		if (extraMessage != "")
			errorMessage += extraMessage + "\n";
		if (userData.isValid())
			errorMessage += "USER_DATA:\n" + userData.toString() + "\n";
		StringEditor msgEdit = "";
		msgEdit.add(Utils::getHexStr(m_errGroup)).add("//");
		msgEdit.add(Utils::getHexStr(m_errCode, true, 8)).add("  ::  ");
		if (_file_name != "")
		{
			msgEdit.add(_file_name).add(" ");
			if (_line_num > 0)
				msgEdit.add("[LINE: ").addi(_line_num).add("]\n");
			else
				msgEdit.add("\n");
		}
		msgEdit.add(errorMessage);
		IOutputHandler* tmpOutHndl = nullptr;
		if (outputHandler != nullptr)
		{
			tmpOutHndl = &Logger::getOutputHandler();
			Logger::setOutputHandler(*outputHandler);
		}
		if (m_errLevel == tErrorLevel::Warning)
		{
			OX_WARN(msgEdit.str());
		}
		else if (m_errLevel == tErrorLevel::Error)
		{
			OX_ERROR(msgEdit.str());
		}
		else if (m_errLevel == tErrorLevel::Fatal)
		{
			OX_FATAL(msgEdit.str());
		}
		if (tmpOutHndl != nullptr)
			Logger::setOutputHandler(*tmpOutHndl);
	}
}
