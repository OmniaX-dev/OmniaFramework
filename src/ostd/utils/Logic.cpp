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

#include "Logic.hpp"
#include "../string/String.hpp"
#ifndef __APPLE__
#include <bits/stdc++.h>
#endif

namespace ostd
{
	bool LogicEvaluator::eval(const String& _exp)
	{
		String exp(_exp);
		exp.replaceAll("true", "1");
		exp.replaceAll("false", "0");
		exp.replaceAll("and", "&");
		exp.replaceAll("or", "+");
		exp.replaceAll("xor", "^");
		exp.replaceAll("nand", "%");
		exp.replaceAll("nor", "*");
		exp.replaceAll("xnor", ">");
		exp.replaceAll("not", "!");
		exp.replaceAll(" ", "");
		exp.replaceAll("!1", "0");
		exp.replaceAll("!0", "1");
		exp.trim();

		std::stack<bool> values;
		std::stack<eLogicOp> ops;
		char c = 0;
		for (u32 i = 0; i < exp.len(); i++)
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
