#include "Signals.hpp"
#include "../data/BaseObject.hpp"
#include "io/Memory.hpp"

namespace ostd
{
	void SignalHandler::init(bool allow_reinit)
	{
		if (m_initialized && !allow_reinit) return;
		m_recievers.clear();
		m_recievers.reserve(__SIGNAL_BUFFER_START_SIZE);
		m_DelegateRecievers.clear();
		m_DelegateRecievers.reserve(__DELEGATE_SIGNALS_BUFFER_START_SIZE);
		m_initialized = true;
	}

	void SignalHandler::handleDelegateSignals(void)
	{
		for (auto& delegate : m_DelegateRecievers)
			emitSignal(delegate.id, Signal::Priority::RealTime, delegate.ud);
		m_DelegateRecievers.clear();
	}

	void SignalHandler::emitSignal(u32 signal_id, u8 prio, BaseObject& userData)
	{
		if (prio == Signal::Priority::Normal)
		{
			SignalHandler::m_DelegateRecievers.push_back({ signal_id, userData });
			return;
		}
		Signal signal { signal_id, userData, prio };
		if (m_recievers.count(signal_id) < 1)
			return;
		for (auto& obj : m_recievers[signal_id])
		{
			obj->__handle_signal(signal);
			if (signal.handled) return;
		}
	}

	void SignalHandler::connect(BaseObject& object, u32 signal_id)
	{
		m_recievers[signal_id].push_back(&object);
	}

	void SignalHandler::disconnect(BaseObject& object, u32 signal_id)
	{
		auto it = m_recievers.find(signal_id);
		if (it == m_recievers.end())
			return;
		STDVEC_REMOVE(it->second, &object);
	}
}
