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
			class VerticalScrollBar : public Widget
			{
				public:
					inline VerticalScrollBar(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					VerticalScrollBar& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseDragged(const Event& event) override;
					void onMousePressed(const Event& event) override;
					void onMouseReleased(const Event& event) override;
					void onUpdate(void) override;

					inline void setx(f32 xx) override {  }
					inline void sety(f32 yy) override {  }
					inline void setw(f32 ww) override {  }
					inline void seth(f32 hh) override {  }

				private:
					void update_thumb(void);
					void set_thumb_y(f32 thumby);
					bool is_mouse_in_thumb(const Vec2& mouse_pos);

				private:
					f32 m_thumbHeight { 0 };
					f32 m_thumbY { 0 };
					Rectangle m_thumbGlobalBounds { 0, 0, 0, 0 };
					bool m_mousePressed { false };
					bool m_mouseDragged { false };
					f32 m_dragGrabOffset { 0 };
					Rectangle m_correctionOffset { 1, 1, 0, 0 };

					Rectangle m_trackBorderRadii { 0, 0, 10, 0 };
					f32 m_thumbBorderRadius { 16 };
					Color m_trackColor { 70, 70, 70 };
					Color m_thumbColor { 120, 120, 120 };
					Color m_thumbBorderColor { 150, 150, 150 };
					bool m_thumbShowBorder { true };
			};
			class HorizontalScrollbar : public Widget
			{
				public:
					inline HorizontalScrollbar(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					HorizontalScrollbar& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseDragged(const Event& event) override;
					void onMousePressed(const Event& event) override;
					void onMouseReleased(const Event& event) override;
					void onUpdate(void) override;

					inline void setx(f32 xx) override {  }
					inline void sety(f32 yy) override {  }
					inline void setw(f32 ww) override {  }
					inline void seth(f32 hh) override {  }

				private:
					void update_thumb(void);
					void set_thumb_x(f32 thumbx);
					bool is_mouse_in_thumb(const Vec2& mouse_pos);

				private:
					f32 m_thumbWidth { 0 };
					f32 m_thumbX { 0 };
					Rectangle m_thumbGlobalBounds { 0, 0, 0, 0 };
					bool m_mousePressed { false };
					bool m_mouseDragged { false };
					f32 m_dragGrabOffset { 0 };
					Rectangle m_correctionOffset { 0, 0, 0, 0 };

					Rectangle m_trackBorderRadii { 0, 0, 10, 10 };
					f32 m_thumbBorderRadius { 16 };
					Color m_trackColor { 70, 70, 70 };
					Color m_thumbColor { 120, 120, 120 };
					Color m_thumbBorderColor { 150, 150, 150 };
					bool m_thumbShowBorder { true };
			};
		}
	}
}
