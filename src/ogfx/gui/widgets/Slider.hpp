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

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			class Slider : public Widget
			{
				public:
					inline Slider(WindowCore& window, bool vertical = false, f32 min = 0.0f, f32 max = 1.0f, f32 step = 0.1f) : Widget({ 0, 0, 0, 0 }, window) { create(vertical, min, max, step); }
					Slider& create(bool vertical, f32 min, f32 max, f32 step);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onMouseReleased(const Event& event) override;
					void onMouseDragged(const Event& event) override;
					void onMouseScrolled(const Event& event) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					inline f32 getValue(void) const { return m_value; }
					inline void setValueChangedCallback(std::function<void(f32 oldValue, f32 newValue)> callback) { callback_onValueChanged = std::move(callback); }

					void enableVertical(bool enable = true);
					inline bool isVertical(void) const { return m_vertical; }
					inline void disableVertical(void) { enableVertical(false); }
					inline bool isHorizontal(void) const { return !isVertical(); }
					inline void enableHorizontal(bool enable = true) { enableVertical(!enable); }
					inline void disableHorizontal(void) { enableHorizontal(false); }

					OSTD_BOOL_PARAM_GETSET_E(MiddleClickShortcut, m_middleClickShortcut);
					OSTD_BOOL_PARAM_GETSET_E(ScrollWheelShortcut, m_enableScrollWheel);
					OSTD_PARAM_GETSET(f32, MinValue, m_min);
					OSTD_PARAM_GETSET(f32, MaxValue, m_max);
					OSTD_PARAM_GETSET(f32, Step, m_step);

					OSTD_PARAM_GETSET(f32, TrackWidth, m_trackWidth);
					OSTD_PARAM_GETSET(Color, TrackColor, m_trackColor);
					OSTD_PARAM_GETSET(Color, TrackProgressColor, m_trackProgressColor);
					OSTD_BOOL_PARAM_GETSET_E(DrawTicks, m_drawTicks);
					OSTD_PARAM_GETSET(f32, StepTickHeight, m_tickHeight);
					OSTD_PARAM_GETSET(Color, StepTickColor, m_tickColor);
					OSTD_PARAM_GETSET(Color, HandleColor, m_handleColor);
					OSTD_PARAM_GETSET(Vec2, HandleSize, m_handleSize);

				private:
					f32 snap_to_step(f32 val);
					f32 mouse_position_to_value(Vec2 mousePosition);
					void set_value(f32 val);

				private:
					f32 m_value { 0.0f };
					std::function<void(f32 oldValue, f32 newValue)> callback_onValueChanged { nullptr };

					f32 m_min { 0.0f };
					f32 m_max { 1.0f };
					f32 m_step { 0.1f };
					bool m_vertical { false };
					bool m_middleClickShortcut { true };
					bool m_enableScrollWheel { true };

					f32 m_trackWidth { 6 };
					Color m_trackColor { "#500000FF" };
					Color m_trackProgressColor { "#AA0000FF" };

					bool m_drawTicks { true };
					f32 m_tickHeight { 4 };
					Color m_tickColor { Colors::Crimson };

					Color m_handleColor { Colors::Crimson };
					Vec2 m_handleSize { 10, 18 };
			};
		}
	}
}
