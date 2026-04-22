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

#include <ogfx/gui/widgets/Widget.hpp>
#include <atomic>

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			class ProgressBar : public Widget
			{
				public:
					inline ProgressBar(WindowCore& window, f32 min = 0.0f, f32 max = 1.0f, f32 start = 0.0f) : Widget({ 0, 0, 0, 0 }, window), m_progress(std::clamp(start, min, max)), m_min(min), m_max(max) { create(); }
					ProgressBar& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void setProgressNormalized(f32 normalized_value);
					void setProgress(f32 value);
					f32 getProgress(void) const;
					f32 getProgressNormalized(void) const;
					OSTD_PARAM_GETSET(Color, ProgressColor, m_progressColor);
					OSTD_BOOL_PARAM_GETSET_E(ShowDecimal, m_showDecimal);
					OSTD_BOOL_PARAM_GETSET_E(ShowText, m_showText);
					OSTD_BOOL_PARAM_GETSET_E(ProgressGradient, m_useProgressGradient);
					OSTD_PARAM_GETSET(ColorGradient, ProgressGradient, m_progressGradient);

				private:
					Color m_progressColor { 255, 255, 255 };
					f32 m_min { 0.0f };
					f32 m_max { 1.0f };
					std::atomic<f32> m_progress;
					bool m_showDecimal { false };
					bool m_showText { true };
					bool m_useProgressGradient { true };
					ColorGradient m_progressGradient {
						{ Colors::Crimson.darkened(0.1f), Colors::Crimson.darkened(0.35f) },
						{ 1.0f }
					};
			};
		}
	}
}
