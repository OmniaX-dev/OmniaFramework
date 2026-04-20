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

#include "math/Geometry.hpp"
#include <ogfx/gui/widgets/Widget.hpp>

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{

			class CheckBox : public Widget
			{
				public:
					inline CheckBox(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
					inline CheckBox(WindowCore& window, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					CheckBox& create(const String& text);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseReleased(const Event& event) override;
					void setText(const String& text);
					void setChecked(bool checked);
					inline String getText(void) const { return m_text; }
					inline Color getCheckBorderColor(void) const { return m_checkBorderColor; }
					inline void setCheckBorderColor(const Color& color) { m_checkBorderColor = color; }
					inline Color getCheckBoxColor(void) const { return m_checkBoxColor; }
					inline void setCheckBoxColor(const Color& color) { m_checkBoxColor = color; }
					inline bool isChecked(void) const { return m_checked; }
					inline void setStateChangedCallback(std::function<void(CheckBox&, bool)> callback) { callback_onStateChanged = callback; }

				private:
					void __update_size(ogfx::BasicRenderer2D& gfx);

				private:
					bool m_checked { false };
					String m_text { "" };
					bool m_textChanged { false };
					f32 m_spacing { 10 };
					Vec2 m_checkSize  { 0, 0 };
					i32 m_checkBorderRadius { 5 };
					i32 m_checkBorderWidth { 1 };
					Color m_checkBorderColor { 255, 255, 255 };
					Color m_checkBoxColor { 255, 255, 255 };

					std::function<void(CheckBox&, bool)> callback_onStateChanged { nullptr };
			};
		}
	}
}
