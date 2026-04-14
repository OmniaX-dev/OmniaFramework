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
					inline CheckBox(WindowCore& window, const ostd::String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					CheckBox& create(const ostd::String& text);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseReleased(const Event& event) override;
					void setText(const ostd::String& text);
					void setChecked(bool checked);
					inline ostd::String getText(void) const { return m_text; }
					inline ostd::Color getCheckBorderColor(void) const { return m_checkBorderColor; }
					inline void setCheckBorderColor(const ostd::Color& color) { m_checkBorderColor = color; }
					inline ostd::Color getCheckBoxColor(void) const { return m_checkBoxColor; }
					inline void setCheckBoxColor(const ostd::Color& color) { m_checkBoxColor = color; }
					inline ostd::Color getTextColor(void) const { return m_textColor; }
					inline void setTextColor(const ostd::Color& color) { m_textColor = color; }
					inline int32_t getFontSize(void) const { return m_fontSize; }
					inline void setFontSize(int32_t fontSize) { m_fontSize = fontSize; }
					inline bool isChecked(void) const { return m_checked; }
					inline void setStateChangedCallback(std::function<void(CheckBox&, bool)> callback) { callback_onStateChanged = callback; }

				private:
					void __update_size(ogfx::BasicRenderer2D& gfx);

				private:
					bool m_checked { false };
					ostd::String m_text { "" };
					bool m_textChanged { false };
					int32_t m_fontSize { 20 };
					ostd::Color m_textColor { 255, 255, 255 };
					float m_spacing { 10 };
					ostd::Vec2 m_checkSize  { 0, 0 };
					int32_t m_checkBorderRadius { 5 };
					int32_t m_checkBorderWidth { 1 };
					ostd::Color m_checkBorderColor { 255, 255, 255 };
					ostd::Color m_checkBoxColor { 255, 255, 255 };

					std::function<void(CheckBox&, bool)> callback_onStateChanged { nullptr };
			};
		}
	}
}
