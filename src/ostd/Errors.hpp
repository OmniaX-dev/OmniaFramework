#ifndef __ERRORS_HPP__
#define __ERRORS_HPP__

#include <ostd/BaseObject.hpp>
#include <ostd/Utils.hpp>
#include <Types.hpp>

namespace ostd
{
	struct tErrorLevel
	{
		inline static constexpr uint8_t NoError = 0x00;
		inline static constexpr uint8_t Warning = 0x01;
		inline static constexpr uint8_t Error = 0x02;
		inline static constexpr uint8_t Fatal = 0x03;
	};

	class RuntimeError : public BaseObject
	{
		public:
			inline RuntimeError(void) { invalidate(); }
			inline RuntimeError(uint8_t group, uint64_t code, uint8_t level, const String& msg) { create(group, code, level, msg); }
			RuntimeError& create(uint8_t group, uint64_t code, uint8_t level, const String& msg);
			
			void fire(const String& extraMessage = "", IOutputHandler* outputHandler = nullptr, BaseObject& userData = BaseObject::InvalidRef(), int32_t _line_num = 0, const String& _file_name = "");

		private:
			uint8_t m_errGroup { 0x00 };
			uint8_t m_errLevel { tErrorLevel::NoError };
			uint64_t m_errCode { 0x0000000000000000 };
			String m_message { "" };

	};
}

#endif
