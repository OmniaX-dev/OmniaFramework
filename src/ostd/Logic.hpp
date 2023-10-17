/**
 * @file Logic.hpp
 * @author OmniaX-dev (https://github.com/OmniaX-dev)
 * @brief The LogicEvaluator class contained in this file, is a simple static utility class
 * 		  used to solve logical expressions in boolean algebra. It only has one public method
 * 		  'eval(...)'. This method takes a String as input, representing the logical expression,
 * 		  and returns a boolean representing the result of the expression.
 * 		  Example of a logical expression:
 * 				1&(0+(1&1))
 * 		  where 1 and 0 represent respectively true and false, '&' represents the AND operation
 * 		  and '+' represents the OR operation. For a more in-depth view of the expression format,
 * 		  and a list of all operations allowed, you can visit the official documentation.
 * 		  Note: The CommandShell includes a '/solve' command out of the box. This command 
 * 		  expects a logical expression as it's only parameter, and returns either true or false as
 * 		  it's output. This command internally uses the LogicEvaluator class.
 * @version 0.1
 * @copyright Copyright (c) 2022 OmniaX-dev
 */


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
