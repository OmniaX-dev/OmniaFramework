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

#include <ogfx/ogfx.hpp>

ostd::ConsoleOutputHandler out;

class Window : public ogfx::GraphicsWindow
{
	public:
		inline Window(void) : m_sigHandler(m_textInput, *this) {  }
		inline void onInitialize(void) override
		{
			connectSignal(ogfx::gui::RawTextInput::actionEventSignalID);

			m_gfx.init(*this);
			m_gfx.setFont("res/ttf/Courier Prime.ttf");

			float w = getWindowWidth();
			float h = 40.0f;
			m_textInput.create({ 0.0f, (float)(getWindowHeight() - h) }, { w, h }, "MainInputTXT");
			m_textInput.setEventListener(m_sigHandler);
			// m_textInput.setCharacterFilter(m_numCharFilter);
			m_textInput.getTheme().extraPaddingTop = 3;
	 	}

		inline void handleSignal(ostd::tSignal& signal) override
		{
			if (signal.ID == ostd::tBuiltinSignals::KeyReleased)
			{
				auto& evtData = (ogfx::KeyEventData&)signal.userData;
				if (evtData.keyCode == SDLK_ESCAPE)
					close();
			}
			if (signal.ID == ogfx::gui::RawTextInput::actionEventSignalID)
			{
				auto& data = (ogfx::gui::RawTextInput::ActionEventData&)signal.userData;
				if (data.senderName != "MainInputTXT")
					return;
				if (data.eventType == ogfx::gui::RawTextInput::eActionEventType::Enter)
				{
					out().fg(ostd::ConsoleColors::Green).p(data.sender.getText()).reset().nl();
					data.sender.setText("");
				}
				else if (data.eventType == ogfx::gui::RawTextInput::eActionEventType::Tab)
				{
					out().fg(ostd::ConsoleColors::Red).p("TAB").reset().nl();
					data.sender.appendText("TAB");
				}
			}
		}

		inline void onRender(void) override
		{
			m_textInput.render(m_gfx);
		}

		inline void onFixedUpdate(double frameTime_s) override
		{
			m_textInput.fixedUpdate();
		}

		inline void onUpdate(void) override
		{
			m_textInput.update();
		}

	private:
		ogfx::gui::RawTextInput m_textInput;
		ogfx::BasicRenderer2D m_gfx;
		ogfx::gui::RawTextInputEventListener m_sigHandler;
		ogfx::gui::RawTextInputNumberCharacterFilter m_numCharFilter;
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
