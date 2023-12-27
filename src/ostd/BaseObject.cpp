#include "BaseObject.hpp"
#include "Utils.hpp"
#include "Defines.hpp"
#include "Signals.hpp"
#include "IOHandlers.hpp"

namespace ostd
{
	BaseObject BaseObject::s_invalid_obj { false };

	BaseObject::BaseObject(const BaseObject& copy)
	{
		m_uid = copy.m_uid;
		m_oid = copy.m_oid;
		m_valid = copy.m_valid;
		m_typeName = copy.m_typeName;
		m_signalsEnabled = copy.m_signalsEnabled;
	}

	BaseObject& BaseObject::operator=(const BaseObject& copy)
	{
		m_uid = copy.m_uid;
		m_oid = copy.m_oid;
		m_valid = copy.m_valid;
		m_typeName = copy.m_typeName;
		m_signalsEnabled = copy.m_signalsEnabled;
		return *this;
	}

	std::ostream& operator<<(std::ostream& out, const BaseObject& val)
	{
		out << val.toString();
		return out;
	}

	void BaseObject::print(bool newLine, OutputHandlerBase* __destination) const
	{
		if (__destination == nullptr)
			std::cout << toString() << (newLine ? "\n" : "");
		else
		{
			__destination->p(toString());
			if (newLine) __destination->nl();
		}
	}

	String BaseObject::getObjectHeaderString(void) const
	{
		return StringEditor(getTypeName()).add("->uid=").addi(getID()).add("/oid=").addi(getCompareOID()).add("/valid=").add(STR_BOOL(isValid())).str();
	}

	void BaseObject::connectSignal(uint32_t signal_id)
	{
		SignalHandler::connect(*this, signal_id);
	}

	void BaseObject::__handle_signal(tSignal& signal)
	{
		if (m_signalsEnabled)
			handleSignal(signal);
	}
} 
