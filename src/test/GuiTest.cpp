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

#include "gui/Events.hpp"
#include <SDL3/SDL_rect.h>
#include <ogfx/utils/Keycodes.hpp>
#include <ogfx/ogfx.hpp>

ostd::ConsoleOutputHandler out;

class Window : public ogfx::gui::Window
{
	public:
		inline Window(void) {  }
		inline void onInitialize(void) override
		{
			// m_img.loadFromFile("./img.png", m_gfx);
			// ogfx::Animation::AnimationData ad;
			// ad.backwards = false;
			// ad.columns = 9;
			// ad.rows = 4;
			// ad.frame_width = 256;
			// ad.frame_height = 256;
			// ad.length = 36;
			// ad.random = false;
			// ad.still = false;
			// ad.speed = 0;
			// m_anim.create(ad);
			// m_anim.setSpriteSheet(m_img);



			m_label1.setText("Show Panel2");
			m_label1.setMousePressedCallback([&](const ogfx::gui::Event& event) -> void {
				m_check1.setChecked(!m_check1.isChecked());
				m_label1.addy(10);
			});
			m_label1.addThemeOverride("@.label.showBackground", true);
			m_label1.addThemeOverride("@.label.backgroundColor", Colors::Beige);
			m_label1.addThemeOverride("@.label.showBorder", true);
			m_label1.addThemeOverride("@.label.borderWidth", 2);
			m_label1.addThemeOverride("@.label.borderColor", Colors::DarkMagenta);
			m_label1.addThemeOverride("@:pressed.label.backgroundColor", Colors::Crimson);
			m_label1.addThemeOverride("@:hover.label.backgroundColor", Colors::DarkRed);
			m_label1.addThemeOverride("@:hover.label.cursor", cast<i32>(ogfx::WindowCore::eCursor::Pointer));
			m_label1.reloadTheme();

			m_check1.setText("Check this out!");
			m_check1.setChecked(true);
			m_check1.setStateChangedCallback([&](ogfx::gui::widgets::CheckBox& sender, bool state) -> void {
				m_panel1.setVisible(state);
			});

			m_label2.setText("Label2");
			m_label3.setText("Label3");

			m_panel1.setSize(300, 140);
			m_panel1.allowScroll(false);

			m_panel2.setSize(600, 400);

			m_scroll.setMargin({ 0, m_panel2.getTitlebarHeight(), 0, 0 });

			m_panel1.addWidget(m_label2);

			m_panel2.addWidget(m_label3);
			m_panel2.addWidget(m_panel1, { 0, 50 });
			m_panel2.addWidget(m_label1, { 0, 500 });
			m_panel2.addWidget(m_scroll);

			addWidget(m_check1, { 30, 30 });
			addWidget(m_panel2, { 30, 100 });

			m_theme.loadFromFile("./DefaultTheme.oss", true, getDefaultStylesheetVariableList());
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
			// gfx.drawAnimation(m_anim, { 200, 200 });
		}

		void onFixedUpdate(void) override
		{
			// m_anim.update();
		}

	private:
		ogfx::gui::widgets::Label m_label1 { *this };
		ogfx::gui::widgets::Label m_label2 { *this };
		ogfx::gui::widgets::Label m_label3 { *this };
		ogfx::gui::widgets::Panel m_panel1 { *this };
		ogfx::gui::widgets::Panel m_panel2 { *this };
		ogfx::gui::widgets::CheckBox m_check1 { *this };
		ogfx::gui::widgets::VerticalScrollBar m_scroll { *this };

		ostd::Stylesheet m_theme;
		ogfx::Animation m_anim;
		ogfx::Image m_img;
};

i32 main(i32 argc, char** argv)
{
	ostd::initialize();
	Window window;
	window.initialize(800, 600, "OmniaFramework - Test Window");
	window.setClearColor({ 0, 0, 0 });
	window.mainLoop();
	return 0;
}
