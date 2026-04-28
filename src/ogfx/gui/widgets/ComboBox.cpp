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

#include "ComboBox.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		ComboBox& ComboBox::create(void)
		{
			setPadding({ 5, 5, 5, 5 });
			setTypeName("ogfx::gui::ComboBox");
			disableChildren();
			setStylesheetCategoryName("combo");
			connectSignal(ostd::BuiltinSignals::MouseReleased);
			validate();
			return *this;
		}

		void ComboBox::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			gfx.outlinedRect(*this, getBackgroundColor(), getBorderColor(), getBorderWidth());

			// Triangle Indicator
			const f32 triPad = getTriangleIndicatorPadding();
			const f32 triSide = geth() - (triPad * 2);
			Triangle tri {
				getGlobalPosition() + Vec2 { getw() - triPad - triSide, triPad },
				getGlobalPosition() + Vec2 { getw() - triPad, triPad },
				getGlobalPosition() + Vec2 { getw() - triPad - (triSide * 0.5f), geth() - triPad }
			};
			gfx.fillTriangle(tri, getTriangleIndicatorColor());
		}

		void ComboBox::onUpdate(void)
		{

		}

		void ComboBox::onMouseReleased(const Event& event)
		{
			if (isMouseInside())
			{
				setThemeQualifier("active", true);
				m_dropDownShown = true;
			}
		}

		void ComboBox::applyTheme(const ostd::Stylesheet& theme)
		{
			setTriangleIndicatorColor(getThemeValue<Color>(theme, "triangleIndicatorColor", getTriangleIndicatorColor()));
			setTriangleIndicatorPadding(getThemeValue<f32>(theme, "triangleIndicatorPadding", getTriangleIndicatorPadding()));
		}

		void ComboBox::handleSignal(ostd::Signal& signal)
		{
			if (signal.ID == ostd::BuiltinSignals::MouseReleased)
			{
				if (isMouseInside())
					return;
				if (!m_dropDownShown)
					return;
				setThemeQualifier("active", false);
			}
		}
	}
}
