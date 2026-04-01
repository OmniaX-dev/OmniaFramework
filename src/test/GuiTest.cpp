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

#include <ogfx/ogfx.hpp>

ostd::ConsoleOutputHandler out;

class Window : public ogfx::gui::Window
{
	public:
		inline Window(void) {  }
		inline void onInitialize(void) override
		{
	 	}

		inline void handleSignal(ostd::tSignal& signal) override
		{
			if (signal.ID == ostd::tBuiltinSignals::KeyReleased)
			{
				auto& evtData = (ogfx::KeyEventData&)signal.userData;
				if (evtData.keyCode == SDLK_ESCAPE)
					close();
			}
		}

		void onRedraw(ogfx::BasicRenderer2D& gfx) override
		{
			wout().fg(ostd::Color { 255, 0, 0 }).p("Hello ").fg(ostd::Color { 0, 0, 255 }).p("World");
		}

	private:
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
