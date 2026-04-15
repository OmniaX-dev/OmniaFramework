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

#include "MathUtils.hpp"
#include <math.h>

namespace ostd
{
	f32 MathUtils::map_value(f32 input, f32 input_start, f32 input_end, f32 output_start, f32 output_end)
	{
		f32 slope = 1.0 * (output_end - output_start) / (input_end - input_start);
		return output_start + round(slope * (input - input_start));
	}
}
