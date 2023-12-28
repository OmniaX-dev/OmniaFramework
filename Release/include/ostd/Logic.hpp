#ifndef __LOGIC_HPP__
#define __LOGIC_HPP__

#include <map>
#include <ostd/Types.hpp>

namespace ostd
{
	enum class eLogicOp
	{
		AND = 0,
		OR,
		NOT,
		XOR,

		NAND,
		NOR,
		XNOR,

		O_PARENTH = 125,
		C_PARENTH = 126,
		NONE = 127
	};
	
	class LogicEvaluator
	{
		public:
			static bool eval(String exp);

		private:
			static bool applyOp(bool a, bool b, eLogicOp op);
			static eLogicOp parseOp(char op);
	};
}

#endif
