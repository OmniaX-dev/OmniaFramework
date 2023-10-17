#ifndef __BASE_OBJECT_HPP__
#define __BASE_OBJECT_HPP__

#include <cstdint>
#include <string>
#include <iostream>

namespace ostd
{
	class IOutputHandler;
	struct tSignal;
	class BaseObject
	{
		public:
			BaseObject(const BaseObject& copy);
			inline virtual ~BaseObject(void) = default;
			virtual BaseObject& operator=(const BaseObject& copy);

			virtual inline uint64_t getID(void) const { return m_uid; }
			virtual inline void setID(uint64_t id) { m_uid = id; }
			
			virtual inline bool isValid(void) const { return !isInvalid(); }
			virtual inline bool isInvalid(void) const { return !m_valid || m_oid == 0; }
			virtual inline void invalidate(void) { m_valid = false; }
			virtual inline void validate(void) { m_valid = true; }
			virtual inline void setValid(bool valid) { m_valid = valid; }

			inline uint64_t getCompareOID(void) const { return m_oid; }
			inline bool compareByOID(const BaseObject& other) const { return m_oid == other.m_oid; }

			inline static BaseObject& InvalidRef(void) { return BaseObject::s_invalid_obj; } 
			// inline static BaseObject InvalidInst(void) { return BaseObject::s_invalid_obj; }
			
			inline void setTypeName(std::string tn) { m_typeName = tn; }
			inline std::string getTypeName(void) const { return m_typeName; }
			std::string getObjectHeaderString(void) const;
			
			inline bool signalsEnabled(void) { return m_signalsEnabled; }
			inline void enableSignals(bool e = true) { m_signalsEnabled = e; }

			virtual inline std::string toString(void) const { return getObjectHeaderString(); };
			virtual void print(bool newLine = true, IOutputHandler* __destination = nullptr) const;

			virtual inline void handleSignal(tSignal& signal) {  }
			void connectSignal(uint32_t signal_id);
			
			void __handle_signal(tSignal& signal);

			friend std::ostream& operator<<(std::ostream& os, const BaseObject& obj);

		protected:
			inline BaseObject(void) { m_uid = -1; m_valid = false; m_oid = BaseObject::s_next_oid++; }
		private:
			inline BaseObject(bool __valid) { m_uid = -1; m_valid = __valid; m_oid = 0; }

		private:
			uint64_t m_uid;
			uint64_t m_oid;
			bool m_valid;
			std::string m_typeName;
			bool m_signalsEnabled { true };

			inline static uint64_t s_next_oid { 1024 };
			static BaseObject s_invalid_obj;
	};
} //namesoace ox



#endif
