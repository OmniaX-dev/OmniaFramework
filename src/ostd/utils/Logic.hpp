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

#include <ostd/string/String.hpp>

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
			static bool eval(const String& exp);

		private:
			static bool applyOp(bool a, bool b, eLogicOp op);
			static eLogicOp parseOp(char op);
	};
}
