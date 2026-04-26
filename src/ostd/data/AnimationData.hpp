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

#include <ostd/data/Types.hpp>
#include <ostd/string/String.hpp>

namespace ostd
{
	struct AnimationData : public ostd::__i_stringeable {
		i32 frameCount { 1 };
		i32 stillFrame { 0 };
		f64 fps { 60.0 };

		i32 rowOffset { 0 };
		i32 columnOffset { 0 };
		f32 pixelOffsetX { 0.0f };
		f32 pixelOffsetY { 0.0f };
		f32 spacingX { 0.0f };;
		f32 spacingY { 0.0f };;
		i32 rows { 1 };
		i32 columns { 1 };
		f32 frameWidth{ 32 };
		f32 frameHeight { 32 };

		bool still { false };
		bool turnBack { false };
		bool random { false };

		inline String toString(void) const override
		{
			String str = "";
			str.add("AnimData {");
			str.add("  ").add("frameCount: ").add(frameCount).add(",\n");
			str.add("  ").add("stillFrame: ").add(stillFrame).add(",\n");
			str.add("  ").add("rowOffset: ").add(rowOffset).add(",\n");
			str.add("  ").add("columnOffset: ").add(columnOffset).add(",\n");
			str.add("  ").add("pixelOffsetX: ").add(pixelOffsetX, 2).add(",\n");
			str.add("  ").add("pixelOffsetY: ").add(pixelOffsetY, 2).add(",\n");
			str.add("  ").add("spacingX: ").add(spacingX, 2).add(",\n");
			str.add("  ").add("CpacingY: ").add(spacingY, 2).add(",\n");
			str.add("  ").add("rows: ").add(rows).add(",\n");
			str.add("  ").add("columns: ").add(columns).add(",\n");
			str.add("  ").add("frameWidth: ").add(frameWidth, 2).add(",\n");
			str.add("  ").add("frameHeight: ").add(frameHeight, 2).add(",\n");
			str.add("  ").add("still: ").add(STR_BOOL(still)).add(",\n");
			str.add("  ").add("turnBack: ").add(STR_BOOL(turnBack)).add(",\n");
			str.add("  ").add("random: ").add(STR_BOOL(random)).add(",\n");
			str.add("}");
			return str;
		}
};
}
