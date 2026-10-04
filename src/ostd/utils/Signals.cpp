#include "Signals.hpp"
#include "../data/Object.hpp"
#include "io/Memory.hpp"
#include <mutex>
#include <variant>
#include <algorithm>

namespace ostd
{
	namespace legacy
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

	// ===================================================== MODERN =====================================================
	namespace
	{
		struct Receiver
		{
			std::variant<Object*, std::weak_ptr<Object>> target;
			Object::SignalCallback callback;
		};

		struct DelegateSignal
		{
			u32 id;
			Object* userData;
		};

		// Deliberately leaked (never destroyed) - any static/global Object (the shared
		// Object::Invalid() sentinel among them) may still run its destructor, which calls
		// disconnectAll(), at an arbitrary point during static deinitialization at program exit.
		// If these were ordinary function-local statics, C++'s reverse-construction-order
		// destruction rule could tear one of them down before that happens, depending purely on
		// which static got touched first at runtime - a classic "static destruction order"
		// crash. Leaking them removes the ordering hazard entirely; the OS reclaims the memory
		// at process exit regardless.
		std::mutex& signal_mutex(void)
		{
			static std::mutex& m = *new std::mutex();
			return m;
		}

		stdumap<u32, stdvec<Receiver>>& signal_receivers(void)
		{
			static stdumap<u32, stdvec<Receiver>>& m = *new stdumap<u32, stdvec<Receiver>>();
			return m;
		}

		stdvec<DelegateSignal>& signal_delegate_queue(void)
		{
			static stdvec<DelegateSignal>& v = *new stdvec<DelegateSignal>();
			return v;
		}

		// Resolves a Receiver's target to a live Object*, locking its weak_ptr (if that's the
		// kind of entry it is) for just the duration of the caller's use - nullptr means the
		// weak_ptr-tracked object has already been destroyed, safe to skip.
		Object* resolve_receiver(const Receiver& r, std::shared_ptr<Object>& lockKeepAlive)
		{
			if (auto* raw = std::get_if<Object*>(&r.target))
				return *raw;
			if (auto* weak = std::get_if<std::weak_ptr<Object>>(&r.target))
			{
				lockKeepAlive = weak->lock();
				return lockKeepAlive.get();
			}
			return nullptr;
		}

		void dispatch(Signal& signal, const stdvec<Receiver>& snapshot)
		{
			for (auto& r : snapshot)
			{
				std::shared_ptr<Object> keepAlive;
				Object* target = resolve_receiver(r, keepAlive);
				if (!target || !target->signalsEnabled())
					continue;
				target->handleSignal(signal);
				if (r.callback)
					r.callback(signal);
				if (signal.handled)
					return;
			}
		}
	}

	void SignalHandler::connect(Object& object, u32 signal_id, Object::SignalCallback cb)
	{
		std::lock_guard<std::mutex> lock(signal_mutex());
		signal_receivers()[signal_id].push_back({ &object, std::move(cb) });
	}

	void SignalHandler::connect(const std::shared_ptr<Object>& object, u32 signal_id, Object::SignalCallback cb)
	{
		if (!object)
			return;
		std::lock_guard<std::mutex> lock(signal_mutex());
		signal_receivers()[signal_id].push_back({ std::weak_ptr<Object>(object), std::move(cb) });
	}

	void SignalHandler::disconnect(Object& object, u32 signal_id)
	{
		std::lock_guard<std::mutex> lock(signal_mutex());
		auto it = signal_receivers().find(signal_id);
		if (it == signal_receivers().end())
			return;
		auto& vec = it->second;
		vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const Receiver& r) {
			if (auto* raw = std::get_if<Object*>(&r.target))
				return *raw == &object;
			if (auto* weak = std::get_if<std::weak_ptr<Object>>(&r.target))
			{
				auto locked = weak->lock();
				return !locked || locked.get() == &object; // opportunistically prune the expired ones too
			}
			return false;
		}), vec.end());
	}

	void SignalHandler::disconnectAll(Object& object)
	{
		std::lock_guard<std::mutex> lock(signal_mutex());
		for (auto& [id, vec] : signal_receivers())
		{
			vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const Receiver& r) {
				if (auto* raw = std::get_if<Object*>(&r.target))
					return *raw == &object;
				return false; // weak_ptr entries self-expire once the shared_ptr is gone; nothing to do here
			}), vec.end());
		}
	}

	void SignalHandler::emitSignal(u32 signal_id, u8 prio, Object* userData)
	{
		if (prio == Signal::Priority::Normal)
		{
			std::lock_guard<std::mutex> lock(signal_mutex());
			signal_delegate_queue().push_back({ signal_id, userData });
			return;
		}

		Signal signal { signal_id, userData, prio };
		stdvec<Receiver> snapshot;
		{
			std::lock_guard<std::mutex> lock(signal_mutex());
			auto it = signal_receivers().find(signal_id);
			if (it == signal_receivers().end())
				return;
			snapshot = it->second;
		}
		dispatch(signal, snapshot);
	}

	void SignalHandler::handleDelegateSignals(void)
	{
		stdvec<DelegateSignal> local;
		{
			std::lock_guard<std::mutex> lock(signal_mutex());
			local.swap(signal_delegate_queue());
		}
		for (auto& d : local)
			emitSignal(d.id, Signal::Priority::RealTime, d.userData);
	}
}
