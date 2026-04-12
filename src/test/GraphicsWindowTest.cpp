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

#include "utils/Keycodes.hpp"
#include <ogfx/ogfx.hpp>

ostd::ConsoleOutputHandler out;

class Window : public ogfx::GraphicsWindow
{
	public:
		inline Window(void) {  }
		inline void onInitialize(void) override
		{
			connectSignal(ogfx::gui::RawTextInput::actionEventSignalID);
			m_gfx.init(*this);
	 	}

		inline void handleSignal(ostd::Signal& signal) override
		{
			if (signal.ID == ostd::BuiltinSignals::KeyReleased)
			{
				auto& evtData = (ogfx::KeyEventData&)signal.userData;
				if (evtData.keyCode == ogfx::KeyCode::Escape)
					close();
			}
		}

		inline void onRender(void) override
		{
			auto l_rndPoint = [&](void) -> ostd::FPoint {
				using rnd = ostd::Random;
				return rnd::getVec2({ 0, (float)getWindowWidth() }, { 0, (float)getWindowHeight() });
			};
			for (int32_t i = 0; i < 10000; i++)
				m_gfx.drawLine({ l_rndPoint(), l_rndPoint() }, ostd::Colors::Crimson, 10);
			m_gfx.endFrame();
		}

		inline void onFixedUpdate(double frameTime_s) override
		{
			std::cout << (int)getFPS() << "\n";
		}

		inline void onUpdate(void) override
		{
		}

	private:
		ogfx::BasicRenderer2D m_gfx;
};

int main(int argc, char** argv)
{
	Window window;
	window.initialize(800, 600, "OmniaFramework - Test Window");
	window.setClearColor({ 0, 2	, 15 });

	while (window.isRunning())
	{
		window.mainLoop();
	}
	return 0;
}
