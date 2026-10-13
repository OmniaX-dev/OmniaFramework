/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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
#include "PianoRollEditor.hpp"

using namespace ogfx::gui;

class TestWindow : public Window
{
	public:
		inline TestWindow(void) {  }
		inline void onInitialize(void) override
		{
			m_theme.loadFromFile("./DefaultThemeDark.oss", true, getDefaultStylesheetVariableList());
			m_theme.loadFromFile("./PianoRollEditorTheme.oss", false, getDefaultStylesheetVariableList());
			setTheme(m_theme);

			getToolBar().setHeight(30);
			getToolBar().setLayout<BoxLayout>(BoxLayout::Orientation::Horizontal);
			getToolBar().getLayout()->setSpacing(10);
			showToolBar();

			auto l_randomIcon = [this]() -> ogfx::Icon {
				ogfx::AnimationData ad;
				ad.frameCount = 1;
				ad.fps = 1;
				ad.columns = 8;
				ad.rows = 5;
				ad.frameWidth = 64;
				ad.frameHeight = 64;
				ad.spacingX = 50;
				ad.spacingY = 48;
				ad.still = true;

				static ogfx::Image img("./icons.png", getGFX());
				ad.columnOffset = ostd::Random::getui32(0, 7);
				ad.rowOffset = ostd::Random::getui32(0, 4);
				if (ad.rowOffset == 3)
					ad.columnOffset = ostd::Random::getui32(0, 4);
				return { img, ad };
			};
			for (i32 i = 0; i < 17; i++)
			{
				auto icon = l_randomIcon();
				auto& btn = getToolBar().addButton("./icons.png");
				btn.enableAnimated();
				btn.setAnimationData(icon);
				btn.setTabIndex(i + 100);
			}

			ostd::SignalHandler::connect(*this, PianoRoll::NoteOnSignal, [this](ostd::Signal& signal) -> void {
				// if (auto* data = dynamic_cast<PianoRollNoteEventData*>(signal.userData))
				//     m_pianoLastNoteLbl.setText(String("Last note: ").add(data->noteInfo.name).add(data->noteInfo.octave));
			});
			ostd::SignalHandler::connect(*this, PianoRoll::MidiStartSignal, [this](ostd::Signal&) -> void {
				out().fg("green").p("[PianoRoll] MidiStart").reset().nl();
			});
			ostd::SignalHandler::connect(*this, PianoRoll::MidiEndSignal, [this](ostd::Signal&) -> void {
				out().fg("yellow").p("[PianoRoll] MidiEnd").reset().nl();
			});

			m_pianoRollEditor.getStyle().loadFromJson("DefaultPianoRollStyle.json");
			m_pianoRollEditor.getStyle().enableShowNoteNamesOnFallingNotes(true);
			m_pianoRollEditor.getStyle().enableShowNoteNamesOnKeys(true);
			// if (!m_pianoRollEditor.loadProject("./audio/Chopin Nocturne No. 20 in C#m.pianoproj"))
			//     out().fg("red").p("[PianoRollEditor] Failed to load project").reset().nl();
			if (m_pianoRollEditor.loadVoice("./audio/Chopin Nocturne Op. 48 No. 1 in Cm.mid", Colors::Transparent, "") == (u32)-1)
				out().fg("red").p("[PianoRoll] Failed to load ./audio/Chopin Nocturne Op. 48 No. 1 in Cm.mid").reset().nl();
			// if (m_pianoRollEditor.loadVoice("./audio/lh.mid", Colors::Blue, "LH") == (u32)-1)
			//     out().fg("red").p("[PianoRoll] Failed to load ./audio/lh.mid").reset().nl();
			// if (m_pianoRollEditor.loadVoice("./audio/rh.mid", Colors::Green, "RH") == (u32)-1)
			//     out().fg("red").p("[PianoRoll] Failed to load ./audio/rh.mid").reset().nl();
			if (!m_pianoRollEditor.loadAudio("./audio/Chopin Nocturne Op. 48 No. 1 in Cm.mp3"))
				out().fg("red").p("[PianoRollEditor] Failed to load ./audio/Chopin Nocturne Op. 48 No. 1 in Cm.mp3").reset().nl();
			m_pianoRollEditor.setMusicKey(ostd::MidiParser::eMusicKey::CMinor);

			m_controlPanel.addThemeID("piano_roll_control_panel");

			addWidget(m_pianoRollEditor, { 0,  0 });
			addWidget(m_controlPanel, { 0, 0 });
			getToolBar().reloadTheme(true);
		}

		inline void onSignal(ostd::Signal& signal) override
		{
			if (signal.ID == ostd::BuiltinSignals::KeyReleased)
			{
				auto& evtData = *(ogfx::KeyEventData*)signal.userData;
				if (evtData.keyCode == ogfx::KeyCode::Escape)
					close();
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseReleased)
			{
			}
			else if (signal.ID == ostd::BuiltinSignals::WindowResized)
			{
				if (!signal.userData)
					return;
				auto& wrd = cast<ogfx::WindowResizedData&>(*signal.userData);
				Vec2 pianoRollSize {
					(1100.0f * wrd.new_width) / 1440.0f, // A tested ratio I like
					wrd.new_height - getToolBar().getHeight()
				};
				m_pianoRollEditor.setPosition({ 0, 0 });
				m_pianoRollEditor.setSize(pianoRollSize);

				Vec2 controlPanelSize {
					wrd.new_width - pianoRollSize.x,
					pianoRollSize.y
				};
				m_controlPanel.setPosition({ pianoRollSize.x, 0 });
				m_controlPanel.setSize(controlPanelSize);
			}
		}

		void onRedraw(ogfx::BasicRenderer2D& gfx) override
		{}

		void onFixedUpdate(void) override
		{
			auto& fm = getFocusManager();
			auto focused = fm.getFocused();
			if (focused)
			{
				if (focused == &m_pianoRollEditor)
					return;
				if (focused->getTypeName() == "ogfx::gui::TextEdit")
					return;
				// Add more exceptions here
			}
			getFocusManager().requestFocus(m_pianoRollEditor, m_rootWidget);
		}

		void onUpdate(f64 delta) override
		{}

	private:
		PianoRollEditor m_pianoRollEditor { *this };
		Panel m_controlPanel { *this };
		ostd::Stylesheet m_theme;
};

i32 main(i32 argc, char** argv)
{
	ostd::Random::autoSeed();
	TestWindow window;
	window.initialize(1400, 900, "MidiEditor - Test");
	window.setClearColor({ 0, 0, 0 });
	window.setPosition({ 50, 50 });
	window.setWindowState(Window::eWindowState::Maximized);
	window.mainLoop();
	return 0;
}
