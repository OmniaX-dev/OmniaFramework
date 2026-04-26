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

#include <ostd/data/AnimationData.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/utils/Time.hpp>
#include <ogfx/resources/Image.hpp>

namespace ogfx
{
	using AnimationData = ostd::AnimationData;
	class Animation
	{
	public:
		inline Animation(void) {  }
		inline Animation(const AnimationData& ad) { create(ad); };
		inline Animation(const AnimationData& ad, Image& spriteSheet) { create(ad, spriteSheet); };
		inline Animation& create(const AnimationData& ad) { return create(ad, InvalidImage); }
		inline void update(void) { m_timer.update(); }
		Animation& create(const AnimationData& ad, Image& spriteSheet);
		void resetAnimation(void);

		inline void setFrameCount(i32 n) { m_animData.frameCount = n; }
		inline void setStillFrame(i32 n) { m_animData.stillFrame = n; }
		inline void setColumnOffset(i32 o) { m_animData.columnOffset = o; }
		inline void setRowOffset(i32 o) { m_animData.rowOffset = o; }
		inline void setPixelOffsetX(f32 o) { m_animData.pixelOffsetX = o; }
		inline void setPixelOffsetY(f32 o) { m_animData.pixelOffsetY = o; }
		inline void setSpacingX(f32 s) { m_animData.spacingX = s; }
		inline void setSpacingY(f32 s) { m_animData.spacingY = s; }
		inline void setNRows(i32 n) { m_animData.rows = n; }
		inline void setNColumns(i32 n) { m_animData.columns = n; }
		inline void setFrameWidth(i32 f) { m_animData.frameWidth = f; }
		inline void setFrameHeight(i32 f) { m_animData.frameHeight = f; }
		inline void enableStill(bool b = true) { m_animData.still = b; }
		inline void enableTurnBack(bool b = true) { m_animData.turnBack = b; }
		inline void enableRandom(bool b = true) { m_animData.random = b; }

		inline i32 getFrameCount(void) { return m_animData.frameCount; }
		inline i32 getStillFrame(void) { return m_animData.stillFrame; }
		inline f64 getFPS(void) { return m_animData.fps; }
		inline i32 getColumnOffset(void) { return m_animData.columnOffset; }
		inline i32 getRowOffset(void) { return m_animData.rowOffset; }
		inline f32 getPixelOffsetX(void) { return m_animData.pixelOffsetX; }
		inline f32 getPixelOffsetY(void) { return m_animData.pixelOffsetY; }
		inline f32 getSpacingX(void) { return m_animData.spacingX; }
		inline f32 getSpacingY(void) { return m_animData.spacingY; }
		inline i32 getNRows(void) { return m_animData.rows; }
		inline i32 getNColumns(void) { return m_animData.columns; }
		inline i32 getFrameWidth(void) { return m_animData.frameWidth; }
		inline i32 getFrameHeight(void) { return m_animData.frameHeight; }
		inline bool isStill(void) { return m_animData.still; }
		inline bool isTurnBackEnabled(void) { return m_animData.turnBack; }
		inline bool isRandomEnabled(void) { return m_animData.random; }

		inline void setSpriteSheet(Image& img) { m_spriteSheet = &img; }
		inline Rectangle getFrameRect(void) const { return m_frameRect; }
		inline const Image& getSpriteSheet(void) const { return (m_spriteSheet != nullptr ? *m_spriteSheet : InvalidImage); }
		inline Image& getSpriteSheet(void) { return (m_spriteSheet != nullptr ? *m_spriteSheet : InvalidImage); }
		inline bool hasImage(void) const { return m_spriteSheet != nullptr; }
		inline const AnimationData& getAnimationData(void) const { return m_animData; }
		inline AnimationData& getAnimationData(void) { return m_animData; }
		inline void removeSpriteSheet(void) { m_spriteSheet = &InvalidImage; }

	private:
		void update_animation(void);

	private:
		inline static Image InvalidImage;
		AnimationData m_animData;
		ostd::StepTimer m_timer;
		Image* m_spriteSheet { nullptr };
		i32 m_currentFrame { 0 };
		bool m_back { false };
		Rectangle m_frameRect { 0, 0, 0, 0 };
	};
}
