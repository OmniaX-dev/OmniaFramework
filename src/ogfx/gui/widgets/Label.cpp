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

#include "Label.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			Label& Label::create(const String& text)
			{
				setText(text);
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Label");
				disableChildren();
				enableBackground(false);
				setStylesheetCategoryName("label");
				validate();
				return *this;
			}

			void Label::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_textChanged)
					__update_size(gfx);
				gfx.drawString(getText(), getGlobalContentPosition(), getTextColor(), getFontSize());
			}

			void Label::setText(const String& text)
			{
				m_text = text;
				m_textChanged = true;
			}

			void Label::__update_size(ogfx::BasicRenderer2D& gfx)
			{
				auto size = gfx.getStringDimensions(getText(), getFontSize());
				size.x += getPadding().left();
				size.x += getPadding().right();
				size.y += getPadding().top();
				size.y += getPadding().bottom();
				setSize({ cast<f32>(size.x), cast<f32>(size.y) });
				m_textChanged = false;
			}
		}
	}
}
