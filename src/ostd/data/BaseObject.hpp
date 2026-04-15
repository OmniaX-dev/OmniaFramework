/*
    OmniaFramework - A collection of useful functionality
    Copyright (C) 2025  OmniaX-Dev

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

#include <cstdint>
#include <ostd/string/String.hpp>
#include <functional>

namespace ostd
{
	class OutputHandlerBase;
	struct Signal;
	class BaseObject : public __i_stringeable
	{
		public: using SignalCallback = std::function<void(Signal&)>;
		public:
			BaseObject(const BaseObject& copy);
			inline virtual ~BaseObject(void) = default;
			virtual BaseObject& operator=(const BaseObject& copy);
			inline void setSignalCallback(SignalCallback callback) { callback_signal = callback; }

			virtual inline u64 getID(void) const { return m_uid; }
			virtual inline void setID(u64 id) { m_uid = id; }

			virtual inline bool isValid(void) const { return !isInvalid(); }
			virtual inline bool isInvalid(void) const { return !m_valid || m_oid == 0; }
			virtual inline void invalidate(void) { m_valid = false; }
			virtual inline void validate(void) { m_valid = true; }
			virtual inline void setValid(bool valid) { m_valid = valid; }

			inline u64 getCompareOID(void) const { return m_oid; }
			inline bool compareByOID(const BaseObject& other) const { return m_oid == other.m_oid; }

			inline static BaseObject& InvalidRef(void) { return BaseObject::s_invalid_obj; }

			inline void setTypeName(const String& tn) { m_typeName = tn; }
			inline String getTypeName(void) const { return m_typeName; }
			String getObjectHeaderString(void) const;

			inline bool signalsEnabled(void) { return m_signalsEnabled; }
			inline void enableSignals(bool e = true) { m_signalsEnabled = e; }

			virtual inline String toString(void) const override { return getObjectHeaderString(); };
			virtual void print(bool newLine = true, OutputHandlerBase* __destination = nullptr) const;

			virtual inline void handleSignal(Signal& signal) {  }
			void connectSignal(u32 signal_id);

			void __handle_signal(Signal& signal);

		protected:
			inline BaseObject(void) { m_uid = -1; m_valid = false; m_oid = BaseObject::s_next_oid++; }
		private:
			inline BaseObject(bool __valid) { m_uid = -1; m_valid = __valid; m_oid = 0; }

		private:
			u64 m_uid;
			u64 m_oid;
			bool m_valid;
			String m_typeName;
			bool m_signalsEnabled { true };
			SignalCallback callback_signal { nullptr };

			inline static u64 s_next_oid { 1024 };
			static BaseObject s_invalid_obj;
	};
}
