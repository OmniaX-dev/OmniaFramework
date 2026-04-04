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




/*
 * Label
 * Button
 * Panel / Container
 * Checkbox
 * Radio Button (Group)
 * Text Input
 * Horizontal Slider
 * Image / Icon
 * ScrollView
 * ListBox
 * ComboBox
 * TreeView
 * Save/Open File Dialogs (and folder dialogs)
 * Tab Panel
 */

#include "utils/Keycodes.hpp"
#include <ogfx/ogfx.hpp>

ostd::ConsoleOutputHandler out;

class Window : public ogfx::gui::Window
{
	public:
		inline Window(void) {  }
		inline void onInitialize(void) override
		{
			m_label1.setPosition(100, 200);
			m_label1.setText("Hello World!");
			m_label1.setMouseMovedCallback([&](const ogfx::gui::Event& event) -> void {
				m_label1.applyThemeValue(m_theme, "label.backgroundColor", ostd::Colors::DarkBlue, false);
				this->setCursor(eCursor::Move);
			});
			addWidget(m_label1);

			m_label2.setPosition(100, 400);
			m_label2.setText("Ciccia Bella!");
			addWidget(m_label2);

			m_theme.set("label.textColor", ostd::Colors::White);
			m_theme.set("label.backgroundColor", ostd::Colors::DarkRed);
			m_theme.set("label.showBackground", true);
			m_theme.set("label.borderRadius", 0);
			setTheme(m_theme);
	 	}

		inline void onSignal(ostd::Signal& signal) override
		{
			if (signal.ID == ostd::BuiltinSignals::KeyReleased)
			{
				auto& evtData = (ogfx::KeyEventData&)signal.userData;
				if (evtData.keyCode == ogfx::KeyCode::Escape)
					close();
			}
		}

		void onRedraw(ogfx::BasicRenderer2D& gfx) override
		{
			// wout().fg(ostd::Color { 255, 0, 0 }).p("Hello ").fg(ostd::Color { 0, 0, 255 }).p("World");
		}

	private:
		ogfx::gui::widgets::Label m_label1 { *this };
		ogfx::gui::widgets::Label m_label2 { *this };
		ogfx::gui::Theme m_theme = ogfx::gui::Theme();
};

int main(int argc, char** argv)
{
	ostd::initialize();
	Window window;
	window.initialize(800, 600, "OmniaFramework - Test Window");
	window.setClearColor({ 0, 0, 0 });
	window.mainLoop();
	return 0;
}
