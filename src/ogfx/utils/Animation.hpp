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
#include <ostd/math/Geometry.hpp>
#include <ogfx/resources/Image.hpp>

namespace ogfx
{
	class Animation
	{
		public: struct AnimationData {
				i32 length;
				i32 speed;
				i32 still_frame;

				i32 row_offset;
				i32 column_offset;

				i32 rows;
				i32 columns;

				i32 frame_width;
				i32 frame_height;

				bool still;
				bool backwards;
				bool random;

				AnimationData(void)
				{
					length = 3;
					speed = 10;
					row_offset = 0;
					column_offset = 0;
					rows = 1;
					columns = 3;
					frame_width = 32;
					frame_height = 32;
					still = false;
					backwards = true;
					random = false;
					still_frame = 1;
				}
			};
	public:
		Animation(void);
		Animation(i32 frames, i32 columns, i32 rows, i32 frame_width, i32 frame_height);
		Animation(AnimationData& ad);
		void create(i32 frames, i32 columns, i32 rows, i32 frame_width, i32 frame_height);
		void create(AnimationData& ad);
		void update(void);
		void resetAnimation(void);

		inline void setFrameNumber(i32 n) { m_frames = n; }
		inline void setColumnNumber(i32 n) { m_columns = n; }
		inline void setBackwards(bool b) { m_backwards = b; }
		inline void setSpeed(i32 d) { m_frame_time = d; }
		inline void setColumnOffset(i32 o) { m_column_offset = o; }
		inline void setRowOffset(i32 o) { m_row_offset = o; }
		inline void setStill(bool s) { m_still = s; }
		inline void setSpriteSheet(Image& img) { m_spriteSheet = &img; }

		inline i32 getFrameNumber(void) const { return m_frames; }
		inline i32 getColumnNumber(void) const { return m_columns; }
		inline bool getBackwards(void) const { return m_backwards; }
		inline i32 getDelay(void) const { return m_frame_time; }
		inline i32 getColumnOffset(void) const { return m_column_offset; }
		inline i32 getRowOffset(void) const { return m_row_offset; }
		inline bool isStill(void) const { return m_still; }
		inline ostd::Rectangle getFrameRect(void) const { return m_frame_rect; }
		inline const Image& getSpriteSheet(void) const { return (m_spriteSheet != nullptr ? *m_spriteSheet : InvalidImage); }
		inline Image& getSpriteSheet(void) { return (m_spriteSheet != nullptr ? *m_spriteSheet : InvalidImage); }
		inline bool hasImage(void) const { return m_spriteSheet != nullptr; }

	private:
		Image* m_spriteSheet { nullptr };
		inline static Image InvalidImage;

		i32 m_frames;
		i32 m_rows;
		i32 m_columns;
		i32 m_frame_width;
		i32 m_frame_height;

		i32 m_column_offset;
		i32 m_row_offset;

		i32 m_frame_time;
		i32 m_current_time;
		i32 m_current_frame;

		bool m_backwards;
		bool m_back;
		bool m_still;
		bool m_random;

		ostd::Rectangle m_frame_rect;
	};
}
