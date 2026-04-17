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
	Animation& Animation::create(const AnimationData& ad, Image& spriteSheet)
	{
		m_animData = ad;
		m_spriteSheet = &spriteSheet;
		m_timer.create(m_animData.fps, [&](f64 dt) -> void {
			update_animation();
		});
		return *this;
	}

	void Animation::resetAnimation(void)
	{
		m_currentFrame = 0;
	}

	void Animation::update_animation(void)
	{
		if (!m_animData.still)
		{
			if (m_animData.random)
			{
				m_currentFrame = ostd::Random::geti32(0, m_animData.frameCount);
			}
			else if (m_animData.turnBack)
			{
				if (!m_back && m_currentFrame >= m_animData.frameCount - 1)
				{
					m_back = true;
					m_currentFrame--;
				}
				else if (!m_back)
					m_currentFrame++;
				else if (m_back && m_currentFrame <= 0)
				{
					m_back = false;
					m_currentFrame++;
				}
				else if (m_back)
					m_currentFrame--;
			}
			else
			{
				if (m_currentFrame++ >= m_animData.frameCount - 1)
					m_currentFrame = 0;
			}
		}
		else if (m_animData.still)
		{
			if (m_animData.stillFrame >= 0 && m_animData.stillFrame < m_animData.frameCount)
				m_currentFrame = m_animData.stillFrame;
			else
				m_currentFrame = 0;
		}
		i32 x = m_animData.pixelOffsetX + (((m_currentFrame % m_animData.columns) + m_animData.columnOffset) * m_animData.frameWidth);
		i32 y = m_animData.pixelOffsetY + (m_animData.rowOffset * m_animData.frameHeight);
		if (m_animData.rows > 1)
			y = m_animData.pixelOffsetY + ((cast<i32>(m_currentFrame / m_animData.columns) + m_animData.rowOffset) * m_animData.frameHeight);
		m_frameRect = { cast<f32>(x), cast<f32>(y), m_animData.frameWidth, m_animData.frameHeight };
	}
}
