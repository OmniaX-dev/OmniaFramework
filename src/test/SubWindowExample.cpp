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

// ============================================================================
// SubWindow example — floating windows inside the main window.
//
//   - "Tools": a regular SubWindow with buttons that open message boxes and
//     re-open the other windows.
//   - "Frosted glass": a SubWindow with the "frosted" theme ID, which turns on
//     the built-in blur (see the (@frosted subwindow) section of the theme).
//   - "Always on top": starts pinned (the pin button in its title bar).
//
// Drag title bars to move, edges/corners to resize, double-click a title bar
// to maximize / restore. Minimized windows line up along the bottom edge.
// The windows stay below the toolbar and the status bar.
// ESC closes a message box, or quits when none is open.
// ============================================================================

#include <ogfx/ogfx.hpp>
#include <cmath>

using namespace ogfx;
using namespace ogfx::gui;

// Something colorful and moving behind the windows, so the blur is easy to see.
class MovingBackground : public Widget
{
	public:
		inline MovingBackground(Window& window) : Widget({ 0, 0, 0, 0 }, window)
		{
			setTypeName("MovingBackground");
			disableFocus();
			validate();
		}

		inline void applyTheme(const ostd::Stylesheet& theme) override
		{
			disableBackground();
			disableBorder();
		}

		inline void onUpdate(void) override { m_time += 1.0f / 60.0f; }

		inline void onDraw(BasicRenderer2D& gfx) override
		{
			const Rectangle b = getGlobalBounds();
			ColorGradient bg({ Color(40, 10, 90), Color(10, 60, 120), Color(90, 20, 60) }, { 1.0f, 1.0f });
			bg.setAngleDeg(m_time * 10.0f);
			// Oversized so the rotating gradient always covers the widget (drawing is clipped to it).
			gfx.fillGradientRect({ b.x - b.w * 0.25f, b.y - b.h * 0.25f, b.w * 1.5f, b.h * 1.5f }, bg);
			const Color palette[] = { Colors::Crimson, Colors::Orange, Colors::Gold, Colors::Lime, Colors::Cyan, Colors::RoyalBlue, Colors::Magenta, Colors::Pink };
			const Vec2 c { b.x + b.w * 0.5f, b.y + b.h * 0.5f };
			for (i32 i = 0; i < 14; i++)
			{
				const f32 a = m_time * (0.3f + i * 0.05f) + i * 0.7f;
				const f32 r = 110.0f + (i % 4) * 70.0f;
				Color col = palette[i % 8];
				col.a = 210;
				gfx.fillCircle({ c.x + std::cos(a) * r * 1.6f, c.y + std::sin(a) * r }, 26.0f + 10.0f * std::sin(m_time * 2.0f + i), col);
			}
			gfx.drawString("OmniaFramework", { b.x + 40, b.y + b.h - 110 }, Colors::White, 72);
		}

	private:
		f32 m_time { 0.0f };
};

class SubWindowExampleWindow : public Window
{
	public:
		inline void onInitialize(void) override
		{
			m_theme.loadFromFile("./DefaultThemeDark.oss", true, getDefaultStylesheetVariableList());
			setTheme(m_theme);

			addWidget(m_background);
			m_background.setAnchorPreset(eAnchorPreset::FullRect);

			// A regular widget, to show the windows float above the widget tree.
			addWidget(m_rootButton);
			m_rootButton.setText("A regular button (click me)");
			m_rootButton.setAnchorPreset(eAnchorPreset::TopRight, ePresetResizeMode::KeepSize, 20);
			m_rootButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				m_rootButton.messageBox("Root widget", "Any widget can open a message box:\n    widget.messageBox(\"Title\", \"Text\");");
			});

			build_tools_window();
			build_frosted_window();
			build_pinned_window();
			build_bars();
		}

	private:
		inline void build_tools_window(void)
		{
			m_tools.setTitle("Tools");
			m_tools.setBounds({ 40, 40, 340, 330 });
			m_tools.addWidget(m_toolsBox);
			m_toolsBox.setAnchorPreset(eAnchorPreset::FullRect);
			m_toolsBox.setSpacing(8);

			m_toolsInfo.setText("Drag, resize, pin, minimize...");
			m_toolsBox.addWidget(m_toolsInfo);

			m_msgBtn.setText("Message box");
			m_msgBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				m_msgBtn.messageBox("Hello", "This is a message box.\nIt's modal: the rest of the window\nignores input until you close it.");
			});
			m_toolsBox.addWidget(m_msgBtn);

			m_iconMsgBtn.setText("Message box with icon");
			m_iconMsgBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				MessageDialog& box = m_iconMsgBtn.messageBox("With an icon", "Same thing, with an ogfx::Icon\nnext to the text.", make_icon(2, 1));
				box.setCloseCallback([this](SubWindow&) { m_toolsInfo.setText("The icon message box was closed"); return true; });
			});
			m_toolsBox.addWidget(m_iconMsgBtn);

			m_reopenBtn.setText("Re-open the other windows");
			m_reopenBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				m_frosted.open();
				m_pinned.open();
			});
			m_toolsBox.addWidget(m_reopenBtn);

			m_blurCheck.setText("Frosted window blur");
			m_blurCheck.setChecked(true);
			m_blurCheck.setStateChangedCallback([this](CheckBox&, bool checked) { m_frosted.enableBlur(checked); });
			m_toolsBox.addWidget(m_blurCheck);

			m_tools.open();
		}

		inline void build_frosted_window(void)
		{
			m_frosted.setTitle("Frosted glass");
			m_frosted.addThemeID("frosted");
			m_frosted.setBounds({ 420, 80, 380, 260 });
			m_frosted.addWidget(m_frostedBox);
			m_frostedBox.setAnchorPreset(eAnchorPreset::FullRect);
			m_frostedBox.setSpacing(8);
			m_frostedLabel.setText("Whatever is behind me is blurred,");
			m_frostedBox.addWidget(m_frostedLabel);
			m_frostedLabel2.setText("including the other windows.");
			m_frostedBox.addWidget(m_frostedLabel2);
			m_frostedSlider.setValueQuiet(16.0f);
			m_frostedSlider.setValueChangedCallback([this](f32, f32 value) { m_frosted.setBlurRadius(value); });
			m_frostedBox.addWidget(m_frostedSlider);
			m_frosted.open();
		}

		inline void build_pinned_window(void)
		{
			m_pinned.setTitle("Always on top");
			m_pinned.setBounds({ 300, 380, 300, 150 });
			m_pinned.addWidget(m_pinnedLabel);
			m_pinnedLabel.setText("Unpin me with the pin button.");
			m_pinnedLabel.setAnchorPreset(eAnchorPreset::Center);
			m_pinned.setAlwaysOnTop(true);
			m_pinned.setButtons(SubWindow::Buttons::Close | SubWindow::Buttons::AlwaysOnTop | SubWindow::Buttons::Minimize);
			m_pinned.open();
		}

		inline void build_bars(void)
		{
			getToolBar().addWidget(m_toolbarLabel);
			m_toolbarLabel.setText("Toolbar: windows stay below it");
			m_toolbarLabel.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 6);
			showToolBar(true);
			getStatusBar().setHeight(28);
			getStatusBar().addWidget(m_statusLabel);
			m_statusLabel.setText("Status bar: and above this one");
			m_statusLabel.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 6);
			showStatusBar(true);
		}

		inline Icon make_icon(i32 column, i32 row)
		{
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
			ad.columnOffset = column;
			ad.rowOffset = row;
			if (!m_icons.isValid())
				m_icons.loadFromFile("./icons.png", getGFX());
			return { m_icons, ad };
		}

	private:
		ostd::Stylesheet m_theme;
		Image m_icons;
		MovingBackground m_background { *this };
		Button m_rootButton { *this };

		SubWindow m_tools { *this };
		VBoxContainer m_toolsBox { *this };
		Label m_toolsInfo { *this };
		Button m_msgBtn { *this };
		Button m_iconMsgBtn { *this };
		Button m_reopenBtn { *this };
		CheckBox m_blurCheck { *this };

		SubWindow m_frosted { *this };
		VBoxContainer m_frostedBox { *this };
		Label m_frostedLabel { *this };
		Label m_frostedLabel2 { *this };
		Slider m_frostedSlider { *this, false, 0.0f, 48.0f, 1.0f };

		SubWindow m_pinned { *this };
		Label m_pinnedLabel { *this };

		Label m_toolbarLabel { *this };
		Label m_statusLabel { *this };
};

i32 main(i32 argc, char** argv)
{
	SubWindowExampleWindow window;
	window.initialize(1280, 800, "OmniaFramework - SubWindow Example");
	window.setClearColor({ 0, 0, 0 });
	window.mainLoop();
	return 0;
}
