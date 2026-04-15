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

#include "Animation.hpp"
#include "../../ostd/math/Random.hpp"

namespace ogfx
{
	Animation::Animation(void)
	{
		create(0, 0, 0, 0, 0);
	}

	Animation::Animation(i32 frames, i32 columns, i32 rows, i32 frame_width, i32 frame_height)
	{
		create(frames, columns, rows, frame_width, frame_height);
	}

	Animation::Animation(AnimationData& ad)
	{
		create(ad);
	}

	void Animation::create(AnimationData& ad)
	{
		create(ad.length, ad.columns, ad.rows, ad.frame_width, ad.frame_height);
		m_backwards = ad.backwards;
		m_still = ad.still;
		m_column_offset = ad.column_offset;
		m_row_offset = ad.row_offset;
		m_frame_time = ad.speed;
		m_random = false;
	}

	void Animation::create(i32 frames, i32 columns, i32 rows, i32 frame_width, i32 frame_height)
	{
		m_frames = frames;
		m_columns = columns;
		m_frame_width = frame_width;
		m_frame_height = frame_height;
		m_rows = rows;
		m_frame_time = 5;
		m_current_time = 0;
		m_current_frame = 0;
		m_backwards = false;
		m_back = false;
		m_still = false;
		m_random = false;
		m_column_offset = 0;
		m_row_offset = 0;
	}

	void Animation::resetAnimation(void)
	{
		m_current_frame = 0;
		m_current_time = 0;
	}

	void Animation::update(void)
	{
		if (m_current_time++ > m_frame_time && !m_still)
		{
			m_current_time = 0;
			if (m_random)
			{
				m_current_frame = ostd::Random::geti32(0, m_frames);
			}
			else if (m_backwards)
			{
				if (!m_back && m_current_frame >= m_frames - 1)
				{
					m_back = true;
					m_current_frame--;
				}
				else if (!m_back)
					m_current_frame++;
				else if (m_back && m_current_frame <= 0)
				{
					m_back = false;
					m_current_frame++;
				}
				else if (m_back)
					m_current_frame--;
			}
			else
			{
				if (m_current_frame++ >= m_frames - 1)
					m_current_frame = 0;
			}
		}
		else if (m_still)
			m_current_frame = 1; //TODO: Hardcoded value
		i32 x = ((m_current_frame % m_columns) + m_column_offset) * m_frame_width;
		i32 y = m_row_offset * m_frame_height;
		if (m_rows > 1)
			y = ((m_current_frame / m_columns) + m_row_offset) * m_frame_height;

		m_frame_rect = { cast<f32>(x), cast<f32>(y), cast<f32>(m_frame_width), cast<f32>(m_frame_height) };
	}
}
