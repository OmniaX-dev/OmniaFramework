#include "Signals.hpp"
#include "../data/BaseObject.hpp"
#include "io/Memory.hpp"

namespace ostd
{
	void SignalHandler::handleDelegateSignals(void)
	{
		for (auto& delegate : delegateReceivers())
			emitSignal(delegate.id, Signal::Priority::RealTime, delegate.ud);
		delegateReceivers().clear();
	}

	void SignalHandler::emitSignal(u32 signal_id, u8 prio, BaseObject& userData)
	{
		if (prio == Signal::Priority::Normal)
		{
			SignalHandler::delegateReceivers().push_back({ signal_id, userData });
			return;
		}
		Signal signal { signal_id, userData, prio };
		if (receivers().count(signal_id) < 1)
			return;
		for (auto& obj : receivers()[signal_id])
		{
			obj->__handle_signal(signal);
			if (signal.handled) return;
		}
	}

	void SignalHandler::connect(BaseObject& object, u32 signal_id)
	{
		receivers()[signal_id].push_back(&object);
	}

	void SignalHandler::disconnect(BaseObject& object, u32 signal_id)
	{
		auto it = receivers().find(signal_id);
		if (it == receivers().end())
			return;
		STDVEC_REMOVE(it->second, &object);
	}
}
