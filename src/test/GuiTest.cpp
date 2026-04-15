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
			m_img.loadFromFile("./img.png", m_gfx);
			ogfx::Animation::AnimationData ad;
			ad.backwards = false;
			ad.columns = 9;
			ad.rows = 4;
			ad.frame_width = 256;
			ad.frame_height = 256;
			ad.length = 36;
			ad.random = false;
			ad.still = false;
			ad.speed = 0;
			m_anim.create(ad);
			m_anim.setSpriteSheet(m_img);

			m_panel1.setSize(300, 140);
			m_panel1.setPosition(350, 50);
			m_panel1.setMousePressedCallback([&](const ogfx::gui::Event& event) -> void {
				pos = { event.mouse->position_x, event.mouse->position_y };
			});
			m_panel1.setMouseDraggedCallback([&](const ogfx::gui::Event& event) -> void {
				if (m_panel1.getPressedMouseButton() == ogfx::MouseEventData::eButton::None)
					return;
				ostd::Vec2 pos2 { event.mouse->position_x, event.mouse->position_y };
				m_panel1.addPos(pos2 - pos);
				pos = pos2;
			});
			m_panel1.setMouseScrolledCallback([&](const ogfx::gui::Event& event) -> void {
				std::cout << "SCROLLED IN THE PANEL LOL \n";
			});


			m_panel2.setSize(600, 400);
			m_panel2.setPosition(170, 200);
			m_panel2.setMousePressedCallback([&](const ogfx::gui::Event& event) -> void {
				ogfx::gui::Widget::setDragAndDropData(m_label3);
				setCursor(eCursor::Move);
			});
			m_panel2.setMouseReleasedCallback([&](const ogfx::gui::Event& event) -> void {
				setCursor(eCursor::Default);
			});

			m_label1.setPosition(100, 200);
			m_label1.setText("Hello World!");
			m_label1.setMousePressedCallback([&](const ogfx::gui::Event& event) -> void {
				std::cout << "PRESS!\n";
				m_check1.setChecked(!m_check1.isChecked());
			});
			m_label1.setMouseScrolledCallback([&](const ogfx::gui::Event& event) -> void {
				std::cout << "SCROLL!\n";
			});
			addWidget(m_label1);
			m_panel2.addChild(m_panel1);
			addWidget(m_panel2);


			m_check1.setPosition(30, 30);
			m_check1.setText("Check this out!");
			m_check1.setStateChangedCallback([&](ogfx::gui::widgets::CheckBox& sender, bool state) -> void {
				std::cout << STR_BOOL(state) << "\n";
			});
			addWidget(m_check1);

			m_label2.setPosition(0, 0);
			m_label2.setText("Ciccia Bella!");
			m_label2.connectSignal(ostd::BuiltinSignals::FileDragAndDropped);
			m_label2.addThemeID("testLabel");
			m_label2.addThemeID("testLabel2");
			m_label2.setSignalCallback([&](ostd::Signal& signal) -> void {
				if (signal.ID == ostd::BuiltinSignals::FileDragAndDropped)
				{
					auto& data = (ogfx::DropEventData&)signal.userData;
					if (ostd::FileSystem::fileExists(data.textOrFilePath))
					{
						m_theme.loadFromFile(data.textOrFilePath);
						m_rootWidget.reloadTheme();
					}
					std::cout << data.textOrFilePath << "\n";
				}
			});
			m_label2.setMouseScrolledCallback([&](const ogfx::gui::Event& event) -> void {
				std::cout << "BARBABARBA!\n";
			});
			m_label2.enableDragAndDrop();
			m_label2.setDragAndDropCallback([&](const ogfx::gui::Event& event) -> void {
				if (event.drop.userObject->getTypeName() == "ogfx::gui::widgets::Label")
				{
					std::cout << cast<ogfx::gui::widgets::Label&>(*event.drop.userObject).getText() << "!\n";
				}
			});
			m_panel1.addChild(m_label2);

			m_label1.addThemeOverride("@:pressed.label.textColor", ostd::Colors::Crimson);

			m_theme.loadFromFile("./testTheme.txt", true, getDefaultStylesheetVariableList());

			m_label2.addThemeOverride("@:hover.label.showBackground", true);
			m_label2.addThemeOverride("@:hover.label.backgroundColor", ostd::Colors::DarkGreen);

			m_label3.setPosition(0, 30);
			m_label3.setText("Bella!");
			m_label3.addThemeID("label3");
			m_panel1.addChild(m_label3);

			wout().setFontSize(80);


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
			gfx.drawAnimation(m_anim, { 200, 200 });
			wout().xy(100, 100).fg(ostd::Colors::Crimson).p("CIAO BELLA").resetColors();
		}

		void onFixedUpdate(void) override
		{
			m_anim.update();
		}

	private:
		ogfx::gui::widgets::Label m_label1 { *this };
		ogfx::gui::widgets::Label m_label2 { *this };
		ogfx::gui::widgets::Label m_label3 { *this };
		ogfx::gui::widgets::Panel m_panel1 { *this };
		ogfx::gui::widgets::Panel m_panel2 { *this };
		ogfx::gui::widgets::CheckBox m_check1 { *this };
		ostd::Stylesheet m_theme;
		ostd::Vec2 pos { 0, 0 };
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
