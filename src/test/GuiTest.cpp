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
#include <ostd/utils/Async.hpp>

ogfx::WindowCore::FileDialogFilterList filters  = {
	{ "Image files", { "png", "jpg", "jpeg", "bmp" } },
	{ "All files", { "*" } }
};

ostd::ConsoleOutputHandler out;
using namespace ogfx::gui::widgets;

class Window : public ogfx::gui::Window
{
	public:
		inline Window(void) {  }
		inline void onInitialize(void) override
		{
			ogfx::AnimationData ad;
			ad.frameCount = 36;
			ad.fps = 60;
			ad.columns = 9;
			ad.rows = 4;
			ad.frameWidth = 256;
			ad.frameHeight = 256;
			m_img.setAnimationData(ad);
			m_img.setImage("./img.png");
			m_img.enableAnimated();
			m_img.setSize(256, 256);

			m_label1.setText("Show Panel2");
			m_label1.setCallback(ogfx::gui::Widget::eCallback::MousePressed, [&](const ogfx::gui::Event& event) -> void {
				m_check1.setChecked(!m_check1.isChecked());
			});
			m_label1.disableTheming();
			m_label1.addThemeOverride("@.label.showBackground", true);
			m_label1.addThemeOverride("@.label.backgroundColor", Colors::Beige);
			m_label1.addThemeOverride("@.label.showBorder", true);
			m_label1.addThemeOverride("@.label.borderWidth", 2);
			m_label1.addThemeOverride("@.label.borderColor", Colors::DarkMagenta);
			m_label1.addThemeOverride("@:pressed.label.backgroundColor", Colors::Crimson);
			m_label1.addThemeOverride("@:hover.label.backgroundColor", Colors::DarkRed);
			m_label1.setBackgroundColor(Colors::Red);
			m_label1.enableBackground();
			m_label1.reloadTheme();

			m_check1.setText("Check this out!");
			m_check1.setChecked(true);
			m_check1.setStateChangedCallback([&](CheckBox& sender, bool state) -> void {
				m_panel1.setVisible(state);
			});

			m_btn1.setText("BUTTON");
			m_btn1.setCallback(ogfx::gui::Widget::eCallback::MousePressed, [&](const ogfx::gui::Event& event) -> void {
				std::cout << showOpenFileDialog(filters) << "\n";
			});
			m_btn1.addThemeOverride("@.button.showIcon", true);
			m_btn1.setAnimationData(ad);
			m_btn1.setIcon("./img.png");
			m_btn1.setIconSize({ 64, 64 });
			m_btn1.enableAnimated();
			m_btn1.enableTooltip();
			m_btn1.setTooltipText("Test tooltip");

			m_label2.setText("Label2");
			m_label3.setText("Label3");
			m_label4.setText("Label4");
			m_label5.setText("Label5");

			m_panel1.setSize(700, 300);
			m_panel1.disableVScroll();
			m_panel1.disableHScroll();
			m_panel1.setTitle("Panel 1");

			m_panel3.setSize(150, 150);
			m_panel3.disableVScroll();
			m_panel3.disableHScroll();
			m_panel3.setTitle("Panel 3");

			m_panel2.setSize(600, 400);
			m_panel2.setTitle("Panel 2");
			m_panel2.enableTooltip();
			m_panel2.setTooltipText("PANEL tooltip");

			m_prog.setSize(300, 30);
			m_slide.setSize(300, 30);
			m_slide.setMinValue(-1.0f);
			// m_slide.enableVertical();
			// m_slide.setStep(0);
			m_slide.setValueChangedCallback([&](f32 oldVal, f32 newVal) -> void {
				m_slideLbl.setText(String("").add(newVal, 2));
			});
			m_slideLbl.setText(String("").add(m_slide.getValue(), 2));

			m_drawCallsLbl.addThemeOverride("@.label.fontSize", 30);
			m_drawCallsLbl.addThemeOverride("@.label.textColor", Colors::Crimson);

			m_list.setSize(200, 300);

			for (i32 i = 0; i < 10000; i++)
			{
				m_list.addLine(ostd::Random::getString(ostd::Random::getui8(1, 40)));
			}
			m_list.addLine("Item 1");
			m_list.addLine("Item 222");
			m_list.addLine("Item 3333333");
			m_list.getLine(10).setFontSize(40);
			m_list.getLine(160).setTextColor(Colors::Crimson);
			m_list.setSelectionChangedCallback([&](stdvec<ogfx::gui::widgets::ListView::Item*>& selection) -> void {\
				std::cout << *(selection[0]) << "\n";
			});

			m_tabs.setSize(900, 700);
			auto& t1 = m_tabs.addTab("Tab1");
			auto& t2 = m_tabs.addTab("Tab2 Test");
			auto& t3 = m_tabs.addTab("Long Tab Test");

			t2.addThemeOverride("@panel_tab.panel.backgroundColor", Colors::SkyBlue);
			t3.addThemeOverride("@panel_tab.panel.backgroundColor", Colors::Orange);

			t1.addWidget(m_check1, { 30, 30 });
			m_radioGroup.addButton(t1, "Radio this out!", { 30, 80 });
			m_radioGroup.addButton(t1, "Radio Opt. 2", { 30, 110 });
			m_radioGroup.addButton(t1, "Radio 3", { 30, 140 });
			m_radioGroup.setSelectionChangedCallback([&](RadioButton& previous, RadioButton& sender) -> void {
				std::cout << previous.getText() << "\n";
				std::cout << sender.getText() << "\n\n";
			});
			t1.addWidget(m_prog, { 30, 200 });
			t1.addWidget(m_slide, { 30, 250 });
			t1.addWidget(m_slideLbl, { 340, 240 });
			t1.addWidget(m_list, { 30, 300 });
			t1.addWidget(m_panel2, { 500, 100 });

			m_panel3.addWidget(m_label4);

			m_panel1.addWidget(m_label2);
			m_panel1.addWidget(m_panel3, { 100, 50 });
			m_panel1.addWidget(m_label5, { 0, 60 });

			m_panel2.addWidget(m_label3);
			m_panel2.addWidget(m_panel1, { 400, 50 });
			m_panel2.addWidget(m_label1, { 0, 500 });
			m_panel2.addWidget(m_btn1, { 0, 300 });
			m_panel2.addWidget(m_img, { 20, 50 });

			addWidget(m_tabs, { 0,  0 });
			addWidget(m_drawCallsLbl, { 39, m_tabs.getSize().y + 30 });

			m_theme.loadFromFile("./DefaultTheme.oss", true, getDefaultStylesheetVariableList());
			setTheme(m_theme);

			m_progressJob.start([this] {
				f32 prog = 0.0f;
				while (prog < 100)
				{
					prog += 0.6f;
					ostd::Time::sleep(ostd::Random::getui32(10, 500));
					this->m_prog.setProgress(prog);
				}
				return true;
			});


			m_menu.onActivate = [this](const ogfx::gui::ContextMenu::Entry& e) {
				std::cout << e.text << "  -  " << (i32)e.id << "\n";
			};

			setContextMenu(m_menu);
		}

		inline void onSignal(ostd::Signal& signal) override
		{
			if (signal.ID == ostd::BuiltinSignals::KeyReleased)
			{
				auto& evtData = (ogfx::KeyEventData&)signal.userData;
				if (evtData.keyCode == ogfx::KeyCode::Escape)
					close();
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseReleased)
			{
				auto& mmd = cast<ogfx::MouseEventData&>(signal.userData);
				if (mmd.button == ogfx::MouseEventData::eButton::Right)
				{
					showContextMenu({ mmd.position_x, mmd.position_y });
				}
			}
		}

		void onRedraw(ogfx::BasicRenderer2D& gfx) override
		{
			// gfx.fillRect(m_tabs.getGlobalPureContentBounds(), { 0, 255, 0, 100 });
			m_drawCallsLbl.setText(String("").add(gfx.getDrawCallCount()));
		}

		void onFixedUpdate(void) override
		{
		}

	private:
		Label m_label1 { *this };
		Label m_label2 { *this };
		Label m_label3 { *this };
		Label m_label4 { *this };
		Label m_label5 { *this };
		Panel m_panel1 { *this };
		Panel m_panel2 { *this };
		Panel m_panel3 { *this };
		CheckBox m_check1 { *this };
		Button m_btn1 { *this };
		ImageLabel m_img { *this };
		TabPanel m_tabs { *this };
		RadioButtonGroup m_radioGroup;
		ProgressBar m_prog { *this, 0, 100 };
		Slider m_slide { *this };
		Label m_slideLbl { *this };
		ListView m_list { *this };
		Label m_drawCallsLbl { *this };

		enum MenuId { New = 1, Open, Save, SaveAs, Exit, CopyRaw, CopyFormatted };

		ogfx::gui::ContextMenu::Instance m_menu { {
			{ "File", -1, {
				{ "New",     MenuId::New },
				{ "Open...", MenuId::Open },
				{ "Save",    MenuId::Save },
				{ "Save As", MenuId::SaveAs },
			}},
			{ "Edit", -1, {
				{ "Copy", -1, {
					{ "As Text", MenuId::CopyRaw },
					{ "As HTML", MenuId::CopyFormatted },
				}},
			}},
			{ "Exit", MenuId::Exit } },
			nullptr
		};

		ostd::StepTimer m_timer;
		ostd::AsyncJob<bool> m_progressJob;

		ostd::Stylesheet m_theme;
};

i32 main(i32 argc, char** argv)
{
	ostd::Random::autoSeed();
	ostd::initialize();
	Window window;
	window.initialize(1200, 800, "OmniaFramework - Test Window");
	window.setClearColor({ 0, 0, 0 });
	window.setPosition({ 50, 50 });
	// window.setWindowState(ogfx::WindowCore::eWindowState::Maximized);
	window.mainLoop();
	return 0;
}
