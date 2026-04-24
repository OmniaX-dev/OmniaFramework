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

#include "Slider.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			Slider& Slider::create(bool vertical, f32 min, f32 max, f32 step)
			{
				setPadding({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::Slider");
				disableChildren();
				disableBackground();
				disableBorder();
				setContentOffset({ 0, 0 });
				setStylesheetCategoryName("slider");
				setMinValue(min);
				setMaxValue(max);
				setStep(step);
				validate();
				return *this;
			}

			void Slider::applyTheme(const ostd::Stylesheet& theme)
			{
				setHandleColor(getThemeValue<Color>(theme, "handle.color", getHandleColor()));
				setHandleSize(getThemeValue<Vec2>(theme, "handle.size", getHandleSize()));
				setTrackColor(getThemeValue<Color>(theme, "track.color", getTrackColor()));
				setTrackProgressColor(getThemeValue<Color>(theme, "track.progressColor", getTrackProgressColor()));
				setTrackWidth(getThemeValue<f32>(theme, "track.width", getTrackWidth()));
				enableDrawTicks(getThemeValue<bool>(theme, "showStepTicks", isDrawTicksEnabled()));
				setStepTickHeight(getThemeValue<f32>(theme, "stepTickHeight", getStepTickHeight()));
				setStepTickColor(getThemeValue<Color>(theme, "stepTickColor", getStepTickColor()));
			}

			void Slider::onMouseReleased(const Event& event)
			{
				if (!isMouseInside())
					return;
				if (isMiddleClickShortcutEnabled() && event.mouse->button == MouseEventData::eButton::Middle)
				{
					set_value(snap_to_step((m_min + m_max) * 0.5f));
					return;
				}
				set_value(mouse_position_to_value({ event.mouse->position_x, event.mouse->position_y }));
			}

			void Slider::onMouseDragged(const Event& event)
			{
				if (!isMouseInside())
					return;
				set_value(mouse_position_to_value({ event.mouse->position_x, event.mouse->position_y }));
			}

			void Slider::onMouseScrolled(const Event& event)
			{
				if (!isMouseInside())
					return;
				if (!isScrollWheelShortcutEnabled())
					return;
				if (getStep() > 0)
				{
					if (event.mouse->scroll == MouseEventData::eScrollDirection::Down ||
						event.mouse->scroll == MouseEventData::eScrollDirection::Right)
						set_value(getValue() + (getStep() * -1));
					else
						set_value(getValue() + getStep());
				}
				else if (event.mouse->scroll == MouseEventData::eScrollDirection::Down ||
						 event.mouse->scroll == MouseEventData::eScrollDirection::Up)
					set_value(getValue() + (0.1f * event.mouse->scrollAmount.y));
				else
					set_value(getValue() + (0.1f * event.mouse->scrollAmount.x));
				event.handle();
			}

			void Slider::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				const auto& bounds = getGlobalContentBounds();
				f32 value = m_value;
				f32 t = (value - m_min) / (m_max - m_min); // normalized [0, 1]

				auto l_centerh = [&](f32 value) -> f32 {
					return (geth() / 2.0f) - (value / 2.0f);
				};
				auto l_centerw = [&](f32 value) -> f32 {
					return (getw() / 2.0f) - (value / 2.0f);
				};

				if (isHorizontal())
				{
					if (m_step > 0.0f && m_drawTicks)
					{
						int tickCount = (int)std::round((m_max - m_min) / m_step) + 1;
						for (int i = 0; i < tickCount; i++)
						{
							f32 v = m_min + i * m_step;
							v = std::min(v, m_max);
							f32 tickT = (v - m_min) / (m_max - m_min);
							f32 tickX = bounds.x + bounds.w * tickT;
							gfx.fillRect({ tickX - 1, bounds.y + l_centerh(getStepTickHeight()) - getStepTickHeight(), 2, getStepTickHeight() }, getStepTickColor());
						}
					}
					Vec2 correction { -2, 0 };
					Vec2 trackPos = bounds.getPosition() + Vec2 { 0, l_centerh(getTrackWidth()) };
					gfx.fillRect({ trackPos + correction, bounds.w + (correction.x * -2), getTrackWidth() }, getTrackColor());
					gfx.fillRect({ trackPos + correction, (bounds.w + (correction.x * -2)) * t, getTrackWidth() }, getTrackProgressColor());
					f32 handleX = bounds.x + bounds.w * t - (getHandleSize().x / 2.0f);
					gfx.fillRect({ handleX, bounds.y + l_centerh(getHandleSize().y), getHandleSize() }, getHandleColor());
				}
				else
				{
					if (m_step > 0.0f && m_drawTicks)
					{
						int tickCount = (int)std::round((m_max - m_min) / m_step) + 1;
						for (int i = 0; i < tickCount; i++)
						{
							f32 v = m_min + i * m_step;
							v = std::min(v, m_max);
							f32 tickT = (v - m_min) / (m_max - m_min);
							f32 tickY = bounds.y + bounds.h * (1.0f - tickT);
							gfx.fillRect({ bounds.x + l_centerw(getStepTickHeight()) - getStepTickHeight(), tickY - 1, getStepTickHeight(), 2 }, getStepTickColor());
						}
					}
					Vec2 correction { 0, -2 };
					Vec2 trackPos = bounds.getPosition() + Vec2 { l_centerw(getTrackWidth()), 0 };
					gfx.fillRect({ trackPos + correction, getTrackWidth(), bounds.h + (correction.y * -2) }, getTrackColor());
					f32 progressH = (bounds.h + (correction.y * -2)) * t;
					gfx.fillRect({ trackPos.x + correction.x, trackPos.y + correction.y + (bounds.h + (correction.y * -2)) - progressH, getTrackWidth(), progressH }, getTrackProgressColor());
					Vec2 swappedSize = getHandleSize().swap();
					f32 handleY = bounds.y + bounds.h * (1.0f - t) - (swappedSize.y / 2.0f);
					gfx.fillRect({ bounds.x + l_centerw(swappedSize.x), handleY, swappedSize }, getHandleColor());
				}
			}

			void Slider::enableVertical(bool enable)
			{
				if (isVertical() && enable) return;
				if (isHorizontal() && !enable) return;
				m_vertical = enable;
				setSize(getSize().swap());
			}

			f32 Slider::snap_to_step(f32 val)
			{
				if (m_step <= 0.0f) return std::clamp(val, m_min, m_max); // full precision
				f32 snapped = std::round((val - m_min) / m_step) * m_step + m_min;
				return std::clamp(snapped, m_min, m_max);
			}

			f32 Slider::mouse_position_to_value(Vec2 mousePosition)
			{
				const auto& bounds = getGlobalBounds();
				f32 raw = isHorizontal() ? (mousePosition.x - bounds.x) / bounds.w : 1.0f - (mousePosition.y - bounds.y) / bounds.h;
				raw = std::clamp(raw, 0.0f, 1.0f);
				return m_min + raw * (m_max - m_min);
			}

			void Slider::set_value(f32 val)
			{
				if (m_value == val)
					return;
				f32 old = m_value;
				m_value = snap_to_step(val);
				if (callback_onValueChanged)
					callback_onValueChanged(old, m_value);
			}
		}
	}
}
