#include "Logic.hpp"
#include "Utils.hpp"
#include <bits/stdc++.h>

namespace ostd
{
	bool LogicEvaluator::eval(String exp)
	{
		StringEditor se(exp);
		se = se.replaceAll("true", "1");
		se = se.replaceAll("false", "0");
		se = se.replaceAll("and", "&");
		se = se.replaceAll("or", "+");
		se = se.replaceAll("xor", "^");
		se = se.replaceAll("nand", "%");
		se = se.replaceAll("nor", "*");
		se = se.replaceAll("xnor", ">");
		se = se.replaceAll("not", "!");
		se = se.replaceAll(" ", "");
		se = se.replaceAll("!1", "0");
		se = se.replaceAll("!0", "1");
		se = se.trim();
		exp = se.str();

		std::stack<bool> values;
		std::stack<eLogicOp> ops;
		char c = 0;
		for (unsigned int i = 0; i < exp.length(); i++)
		{
			c = exp[i];
			if (c == ' ' || c == '#') continue;
			else if (c == '1' || c == '0') values.push(c == '1');
			else if (c == ')')
			{
				while(!ops.empty() && ops.top() != eLogicOp::O_PARENTH)
				{
					bool val2 = values.top();
					values.pop();
					bool val1 = values.top();
					values.pop();
					eLogicOp op = ops.top();
					ops.pop();
					values.push(applyOp(val1, val2, op));
				}
				if(!ops.empty()) ops.pop();
				if (!ops.empty())
				{
					if (ops.top() == eLogicOp::NOT)
					{
						ops.pop();
						bool tmp = values.top();
						values.pop();
						values.push(!tmp);
					}
				}
			}
			else ops.push(parseOp(c));
		}
		while(!ops.empty())
		{
			bool val2 = values.top();
			values.pop();
			bool val1 = values.top();
			values.pop();
			eLogicOp op = ops.top();
			ops.pop();
			values.push(applyOp(val1, val2, op));
		}
		return values.top();
	}

	bool LogicEvaluator::applyOp(bool a, bool b, eLogicOp op)
	{
		switch(op)
		{
			case eLogicOp::AND: return (a && b);
			case eLogicOp::OR: return (a || b);
			case eLogicOp::NAND: return !(a && b);
			case eLogicOp::NOR: return !(a || b);
			case eLogicOp::XOR: return (a != b);
			case eLogicOp::XNOR: return !(a != b);
			default: return false;
		}
	}

	eLogicOp LogicEvaluator::parseOp(char op)
	{
		if (op == '&') return eLogicOp::AND;
		if (op == '+') return eLogicOp::OR;
		if (op == '^') return eLogicOp::XOR;
		if (op == '%') return eLogicOp::NAND;
		if (op == '*') return eLogicOp::NOR;
		if (op == '>') return eLogicOp::XNOR;
		if (op == '!') return eLogicOp::NOT;
		if (op == '(') return eLogicOp::O_PARENTH;
		if (op == ')') return eLogicOp::C_PARENTH;
		return eLogicOp::NONE;
	}
}
