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
#include "../Window.hpp"

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
			connectSignal(ostd::BuiltinSignals::MouseMoved);
			validate();
			return *this;
		}

		i32 ComboBox::addMenuItem(const String& text)
		{
			m_menu.entries.push_back({ text, m_nextItemID++ });
			if (m_menu.entries.size() == 1)
			{
				auto& entry = m_menu.entries.back();
				m_selectedEntry = entry;
				m_selectedEntryText = get_display_text(entry.text);
			}
			return m_nextItemID - 1;
		}

		bool ComboBox::removeMenuItem(const String& text)
		{
			return m_menu.removeByText(text, false);
		}

		bool ComboBox::removeMenuItem(i32 id)
		{
			return m_menu.removeById(id, false);
		}

		void ComboBox::setItemCallback(ContextMenu::Instance::Callback callback)
		{
			m_menu.onActivate = [this, cb = std::move(callback)] (const ContextMenu::Entry& entry) -> void {
				m_selectedEntry = entry;
				m_selectedEntryText = get_display_text(entry.text); // Caching because get_display_text is expensive
				if (cb) cb(entry);
			};
		}

		void ComboBox::setw(f32 w)
		{
			Rectangle::setw(w);
			m_menu.minWidth = getw();
			if (m_selectedEntry.id >= 0)
				m_selectedEntryText = get_display_text(m_selectedEntry.text);
		}

		void ComboBox::applyTheme(const ostd::Stylesheet& theme)
		{
			setTriangleIndicatorColor(getThemeValue<Color>(theme, "triangleIndicatorColor", getTriangleIndicatorColor()));
			setTriangleIndicatorPadding(getThemeValue<f32>(theme, "triangleIndicatorPadding", getTriangleIndicatorPadding()));
		}

		void ComboBox::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			gfx.outlinedRect(*this, getBackgroundColor(), getBorderColor(), getBorderWidth());

			auto bounds = getGlobalBounds();
			gfx.drawCenteredString(m_selectedEntryText, bounds - Rectangle { 0, 0, geth(), 0 }, getTextColor(), getFontSize());

			// Triangle Indicator
			const f32 triPad = getTriangleIndicatorPadding();
			const f32 triSide = geth() - (triPad * 2);
			Triangle tri {
				bounds.getPosition() + Vec2 { getw() - triPad - triSide, triPad },
				bounds.getPosition() + Vec2 { getw() - triPad, triPad },
				bounds.getPosition() + Vec2 { getw() - triPad - (triSide * 0.5f), geth() - triPad }
			};
			gfx.fillTriangle(tri, getTriangleIndicatorColor());
		}

		void ComboBox::onMouseReleased(const Event& event)
		{
			if (contains(event.mouse->position_x, event.mouse->position_y))
			{
				if (m_dropDownShown)
				{
					setThemeQualifier("active", false);
					m_dropDownShown = false;
					return;
				}
				setThemeQualifier("active", true);
				m_dropDownShown = true;
				Vec2 anchor = getGlobalPosition() + Vec2 { 0, geth() };
				cast<Window&>(getWindow()).showContextMenu(m_menu, anchor);
			}
		}

		void ComboBox::handleSignal(ostd::Signal& signal) // This is to get the global events, in order to bypass
														  // the fact that some components like MenuBar call
														  // event.handle() at the very beginning of the event chain
		{
			if (signal.ID == ostd::BuiltinSignals::MouseReleased)
			{
				auto& med = cast<MouseEventData&>(signal.userData);
				if (contains(med.position_x, med.position_y))
					return;
				if (!m_dropDownShown)
					return;
				setThemeQualifier("active", false);
				reloadTheme();
				m_dropDownShown = false;
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseMoved)
			{
				auto& med = cast<MouseEventData&>(signal.userData);
				if (contains(med.position_x, med.position_y))
					return;
				setThemeQualifier("hover", false);
			}
		}

		String ComboBox::get_display_text(const String& entry)
		{
			auto& gfx = getWindow().getGFX();
			f32 _w = getw() - geth();  // Reserve space for HxH triangle indicator

			// Fast path: full text fits.
			if (gfx.getStringDimensions(entry, getFontSize()).x <= _w)
				return entry;

			// Reserve space for the ellipsis.
			f32 ellipsisW = gfx.getStringDimensions("...", getFontSize()).x;
			f32 budget = _w - ellipsisW;

			if (budget <= 0)
				return "";  // Not even room for "..." — degenerate but possible if the combo is tiny

			// Binary search for the largest prefix length that fits within `budget`.
			// Invariant: any length <= lo fits; any length > hi does not.
			size_t lo = 0;
			size_t hi = entry.len();  // adjust to your String's length method

			while (lo < hi)
			{
				size_t mid = lo + (hi - lo + 1) / 2;  // upper-mid, avoids infinite loop when hi == lo+1
				String prefix = entry.new_substr(0, mid);
				if (gfx.getStringDimensions(prefix, getFontSize()).x <= budget)
					lo = mid;     // mid fits, try longer
				else
					hi = mid - 1; // mid doesn't fit, must be shorter
			}

			return entry.new_substr(0, lo) + "...";
		}
	}
}
