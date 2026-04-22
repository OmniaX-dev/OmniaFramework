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

#include "ProgressBar.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			ProgressBar& ProgressBar::create(void)
			{
				setPadding({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::ProgressBar");
				disableChildren();
				enableBackground();
				enableBorder();
				setStylesheetCategoryName("progressbar");
				validate();
				return *this;
			}

			void ProgressBar::applyTheme(const ostd::Stylesheet& theme)
			{
				setProgressColor(getThemeValue<Color>(theme, "progressColor", getProgressColor()));
				enableShowDecimal(getThemeValue<bool>(theme, "showDecimal", isShowDecimalEnabled()));
				enableShowText(getThemeValue<bool>(theme, "showText", isShowTextEnabled()));
				enableProgressGradient(getThemeValue<bool>(theme, "useProgressGradient", isProgressGradientEnabled()));
				setProgressGradient(getThemeValue<ColorGradient>(theme, "progressGradient", getProgressGradient()));
			}

			void ProgressBar::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				f32 prog = getProgressNormalized();
				f32 progressw = (prog * getw());
				if (isProgressGradientEnabled())
					gfx.fillGradientRect({ getGlobalPosition(), { progressw, geth() } }, m_progressGradient);
				else
					gfx.fillRect({ getGlobalPosition(), { progressw, geth() } }, getProgressColor());
				if (isShowTextEnabled())
				{
					String text = "";
					if (isShowDecimalEnabled())
						text.add(prog * 100, 2).add("%");
					else
						text.add(cast<i32>(std::round(prog * 100))).add("%");
					gfx.drawCenteredString(text, getGlobalBounds(), getTextColor(), getFontSize());
				}
			}

			void ProgressBar::setProgressNormalized(f32 normalized_value)
			{
				normalized_value = std::clamp(normalized_value, 0.0f, 1.0f);
				f32 next = m_min + normalized_value * (m_max - m_min);
				m_progress.store(next, std::memory_order_relaxed);
			}

			void ProgressBar::setProgress(f32 value)
			{
				f32 current = m_progress.load(std::memory_order_relaxed);
				f32 next = current;
				next = std::clamp(value, m_min, m_max);
				m_progress.store(next, std::memory_order_relaxed);
			}

			f32 ProgressBar::getProgress(void) const
			{
				return m_progress.load(std::memory_order_relaxed);
			}

			f32 ProgressBar::getProgressNormalized(void) const
			{
				f32 s = m_progress.load(std::memory_order_relaxed);
				return (s - m_min) / (m_max - m_min);
			}
		}
	}
}
