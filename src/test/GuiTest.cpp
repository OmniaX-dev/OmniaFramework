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
 * --- Label
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

#include <ogfx/utils/Keycodes.hpp>
#include <ogfx/ogfx.hpp>

ostd::ConsoleOutputHandler out;

class Window : public ogfx::gui::Window
{
	public:
		inline Window(void) {  }
		inline void onInitialize(void) override
		{
			m_panel1.setSize(300, 140);
			m_panel1.setPosition(200, 150);
			m_panel1.setMousePressedCallback([&](const ogfx::gui::Event& event) -> void {
				pos = { event.mouse->position_x, event.mouse->position_y };
			});
			m_panel1.setMouseDraggedCallback([&](const ogfx::gui::Event& event) -> void {
				ostd::Vec2 pos2 { event.mouse->position_x, event.mouse->position_y };
				m_panel1.addPos(pos2 - pos);
				pos = pos2;
			});

			m_label1.setPosition(100, 200);
			m_label1.setText("Hello World!");
			m_label1.setMousePressedCallback([&](const ogfx::gui::Event& event) -> void {
				std::cout << "PRESS!\n";
			});
			addWidget(m_label1);
			addWidget(m_panel1);
			// m_label1.setThemeQualifier("disabled");

			m_label2.setPosition(10, 10);
			m_label2.setText("Ciccia Bella!");
			m_label2.addThemeID("testLabel");
			m_label2.addThemeID("testLabel2");
			m_panel1.addChild(m_label2);

			m_label1.addThemeOverride("@:pressed.label.textColor", ostd::Colors::Crimson);

			m_theme.loadFromFile("./testTheme.txt");
			setTheme(m_theme);

			m_label2.addThemeOverride("@:hover.label.showBackground", true);
			m_label2.addThemeOverride("@:hover.label.backgroundColor", ostd::Colors::DarkGreen);
			m_theme.debugPrint();
			m_label2.reloadTheme();

			m_theme.debugPrint();
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
		ogfx::gui::widgets::Panel m_panel1 { *this };
		ostd::Stylesheet m_theme;
		ostd::Vec2 pos { 0, 0 };
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
