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
// Layout example — a tour of the anchor-based layout system.
//
// Resize the window to see everything adapt. The window is split into:
//
//   +-----------+---------------------------------------------+
//   | Sidebar   | Main panel                                  |
//   | (25% of   |  [ GridContainer: a small form            ] |
//   |  width,   |  [ Anchor playground: corner/center anchors] |
//   |  percent  |  [ Items: scrollable VBox inside a Panel   ] |
//   |  anchors) |  [ HBox footer: Cancel ....... Apply  OK   ] |
//   +-----------+---------------------------------------------+
//   | status bar: left label            right-anchored label |
//   +----------------------------------------------------------+
//
// Concepts shown:
//   - setRelativeBounds():    percentage-based placement (sidebar / main panel)
//   - setAnchorPreset():      corners, center, full-rect with margins
//   - setAnchors()/setOffsets(): a custom "middle 50%" anchor
//   - VBox/HBox/Grid/Center containers, size flags and stretch ratios
//   - minimum sizes and grow directions
// ============================================================================

#include <ogfx/ogfx.hpp>
#include <memory>

using namespace ogfx;
using namespace ogfx::gui;

class LayoutExampleWindow : public Window
{
	public:
		inline LayoutExampleWindow(void) {  }

		inline void onInitialize(void) override
		{
			m_theme.loadFromFile("./DefaultThemeDark.oss", true, getDefaultStylesheetVariableList());
			setTheme(m_theme);

			build_sidebar();
			build_main_panel();
			build_status_bar();
			apply_split();
		}

		inline void onSignal(ostd::Signal& signal) override
		{
			if (signal.ID == ostd::BuiltinSignals::KeyReleased)
			{
				auto& evtData = *(ogfx::KeyEventData*)signal.userData;
				if (evtData.keyCode == ogfx::KeyCode::Escape)
					close();
			}
			else if (signal.ID == ostd::BuiltinSignals::WindowResized)
			{
				// Nothing to lay out by hand: anchors take care of it. We only update
				// the label (which, being right-anchored, grows towards the left).
				m_sizeLbl.setText(String("Window: ").add(getWindowWidth()).add(" x ").add(getWindowHeight()));
			}
		}

	private:
		// ----------------------------------------------------------------------
		// Sidebar: a Panel placed with percentages, holding a VBoxContainer.
		// ----------------------------------------------------------------------
		void build_sidebar(void)
		{
			m_sidebar.setTitle("Sidebar");
			addWidget(m_sidebar);

			// The box fills the panel's content area with an 8px margin.
			m_sidebar.addWidget(m_sideBox);
			m_sideBox.setAnchorPreset(eAnchorPreset::FullRect, ePresetResizeMode::KeepSize, 8);
			m_sideBox.setSpacing(8);

			m_sideTitle.setText("Controls");
			m_sideBox.addWidget(m_sideTitle);

			m_wideCheck.setText("Wide sidebar (35%)");
			m_wideCheck.setStateChangedCallback([this](CheckBox&, bool checked) {
				m_sidebarFraction = checked ? 0.35f : 0.25f;
				apply_split();
			});
			m_sideBox.addWidget(m_wideCheck);

			// Fill (default) on the horizontal axis: the slider spans the box width.
			m_sideBox.addWidget(m_slider);
			m_slider.setValueChangedCallback([this](f32, f32 newValue) {
				m_progress.setProgressNormalized(newValue);
			});
			m_progress.enableShowText();
			m_sideBox.addWidget(m_progress);

			m_addBtn.setText("Add item");
			m_addBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { add_item(); });
			m_sideBox.addWidget(m_addBtn);

			m_removeBtn.setText("Remove item");
			m_removeBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { remove_item(); });
			m_sideBox.addWidget(m_removeBtn);

			// An expanding, empty spacer pushes the next widget to the bottom.
			m_sideSpacer.setVSizeFlags(eSizeFlags::ExpandFill);
			m_sideBox.addWidget(m_sideSpacer);

			// ShrinkCenter: keep the button at its minimum width, centered.
			m_statusToggleBtn.setText("Toggle status bar");
			m_statusToggleBtn.setHSizeFlags(eSizeFlags::ShrinkCenter);
			m_statusToggleBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				m_statusVisible = !m_statusVisible;
				showStatusBar(m_statusVisible);
			});
			m_sideBox.addWidget(m_statusToggleBtn);
		}

		// ----------------------------------------------------------------------
		// Main panel: a VBoxContainer with a form, an anchor playground, a
		// scrollable list and a footer.
		// ----------------------------------------------------------------------
		void build_main_panel(void)
		{
			m_mainPanel.setTitle("Main panel");
			addWidget(m_mainPanel);

			m_mainPanel.addWidget(m_mainBox);
			m_mainBox.setAnchorPreset(eAnchorPreset::FullRect, ePresetResizeMode::KeepSize, 10);
			m_mainBox.setSpacing(10);

			// --- 1) GridContainer form: labels keep their size, fields expand. ---
			m_form.setColumns(2);
			m_form.setSpacing(12, 6);
			auto l_formRow = [this](Label& lbl, const String& text, Widget& field) {
				lbl.setText(text);
				lbl.setHSizeFlags(eSizeFlags::ShrinkEnd);  // right-aligned in its column
				field.setHSizeFlags(eSizeFlags::ExpandFill);
				m_form.addWidget(lbl);
				m_form.addWidget(field);
			};
			l_formRow(m_nameLbl, "Name", m_nameEdit);
			l_formRow(m_emailLbl, "Email", m_emailEdit);
			m_roleCombo.addMenuItem("Developer");
			m_roleCombo.addMenuItem("Designer");
			m_roleCombo.addMenuItem("Manager");
			l_formRow(m_roleLbl, "Role", m_roleCombo);
			m_mainBox.addWidget(m_form);

			// --- 2) Anchor playground: takes 2/3 of the leftover height. ---
			m_playground.setTitle("Anchor playground (resize the window)");
			m_playground.setVSizeFlags(eSizeFlags::ExpandFill);
			m_playground.setStretchRatio(2.0f);
			// Below this height the corner buttons would overlap. Once the box can't
			// honor all minimum sizes it grows past the panel, which then scrolls.
			m_playground.setMinSize(0, 200);
			m_mainBox.addWidget(m_playground);

			auto l_corner = [this](Button& btn, const String& text, eAnchorPreset preset) {
				btn.setText(text);
				m_playground.addWidget(btn);
				// Auto-sized buttons: the preset keeps their size and sets the grow
				// direction so they stay glued to their corner.
				btn.setAnchorPreset(preset, ePresetResizeMode::KeepSize, 10);
			};
			l_corner(m_tlBtn, "Top left", eAnchorPreset::TopLeft);
			l_corner(m_trBtn, "Top right", eAnchorPreset::TopRight);
			l_corner(m_blBtn, "Bottom left", eAnchorPreset::BottomLeft);
			l_corner(m_brBtn, "Bottom right", eAnchorPreset::BottomRight);

			m_centerLbl.setText("Centered (eAnchorPreset::Center)");
			m_playground.addWidget(m_centerLbl);
			m_centerLbl.setAnchorPreset(eAnchorPreset::Center);

			// Custom anchors: always spans the middle 50% of the width, 55% down,
			// 24px tall.
			m_playground.addWidget(m_halfSlider);
			m_halfSlider.setAnchors(0.25f, 0.55f, 0.75f, 0.55f);
			m_halfSlider.setOffsets(0, 0, 0, 24);

			// --- 3) Scrollable list: a VBox whose height follows its content. ---
			m_itemsPanel.setTitle("Items");
			m_itemsPanel.setVSizeFlags(eSizeFlags::ExpandFill);
			m_itemsPanel.setStretchRatio(1.0f);
			m_itemsPanel.setMinSize(0, 140);
			m_mainBox.addWidget(m_itemsPanel);

			// TopWide + MinSize: full width, height = minimum size of the box, so
			// the box grows with its items and the Panel starts scrolling.
			m_itemsPanel.addWidget(m_itemsBox);
			m_itemsBox.setSpacing(2);
			m_itemsBox.setAnchorPreset(eAnchorPreset::TopWide, ePresetResizeMode::MinSize, 4);
			for (i32 i = 0; i < 5; i++)
				add_item();

			// --- 4) Footer: an HBox with an expanding spacer. ---
			m_cancelBtn.setText("Cancel");
			m_footer.addWidget(m_cancelBtn);
			m_footerSpacer.setHSizeFlags(eSizeFlags::ExpandFill);
			m_footer.addWidget(m_footerSpacer);
			m_applyBtn.setText("Apply");
			m_footer.addWidget(m_applyBtn);
			m_okBtn.setText("OK");
			m_okBtn.setMinSize(80, 0);  // a minimum width makes short labels look consistent
			m_footer.addWidget(m_okBtn);
			m_footer.setSpacing(8);
			m_mainBox.addWidget(m_footer);
		}

		// ----------------------------------------------------------------------
		// Status bar: one label on the left, one anchored to the right edge.
		// ----------------------------------------------------------------------
		void build_status_bar(void)
		{
			getStatusBar().setHeight(28);
			showStatusBar(m_statusVisible);

			m_statusLbl.setText("Anchors + containers demo. Press ESC to quit.");
			getStatusBar().addWidget(m_statusLbl);
			m_statusLbl.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 4);

			m_sizeLbl.setText("Window: ");
			getStatusBar().addWidget(m_sizeLbl);
			m_sizeLbl.setAnchorPreset(eAnchorPreset::CenterRight, ePresetResizeMode::KeepSize, 8);
		}

		// Percentage-based split between sidebar and main panel.
		void apply_split(void)
		{
			const f32 gap = 4.0f;
			m_sidebar.setRelativeBounds({ 0.0f, 0.0f, m_sidebarFraction, 1.0f }, { 8, 8, gap, 8 });
			m_mainPanel.setRelativeBounds({ m_sidebarFraction, 0.0f, 1.0f - m_sidebarFraction, 1.0f }, { gap, 8, 8, 8 });
		}

		void add_item(void)
		{
			m_items.push_back(std::make_unique<Label>(*this, String("Item #").add(++m_itemCounter)));
			m_itemsBox.addWidget(*m_items.back());
			// New widgets need the theme; reloading it for the list is enough.
			m_items.back()->reloadTheme(true);
		}

		void remove_item(void)
		{
			if (m_items.empty())
				return;
			m_itemsBox.removeWidget(*m_items.back());
			m_items.pop_back();
		}

	private:
		ostd::Stylesheet m_theme;

		// Sidebar
		Panel m_sidebar { *this };
		VBoxContainer m_sideBox { *this };
		Label m_sideTitle { *this };
		CheckBox m_wideCheck { *this };
		Slider m_slider { *this, false, 0.0f, 1.0f, 0.01f };
		ProgressBar m_progress { *this };
		Button m_addBtn { *this };
		Button m_removeBtn { *this };
		Label m_sideSpacer { *this };
		Button m_statusToggleBtn { *this };
		f32 m_sidebarFraction { 0.25f };
		bool m_statusVisible { true };

		// Main panel
		Panel m_mainPanel { *this };
		VBoxContainer m_mainBox { *this };

		GridContainer m_form { *this };
		Label m_nameLbl { *this };
		TextEdit m_nameEdit { *this };
		Label m_emailLbl { *this };
		TextEdit m_emailEdit { *this };
		Label m_roleLbl { *this };
		ComboBox m_roleCombo { *this };

		Panel m_playground { *this };
		Button m_tlBtn { *this };
		Button m_trBtn { *this };
		Button m_blBtn { *this };
		Button m_brBtn { *this };
		Label m_centerLbl { *this };
		Slider m_halfSlider { *this };

		Panel m_itemsPanel { *this };
		VBoxContainer m_itemsBox { *this };
		stdvec<std::unique_ptr<Label>> m_items;
		i32 m_itemCounter { 0 };

		HBoxContainer m_footer { *this };
		Button m_cancelBtn { *this };
		Label m_footerSpacer { *this };
		Button m_applyBtn { *this };
		Button m_okBtn { *this };

		// Status bar
		Label m_statusLbl { *this };
		Label m_sizeLbl { *this };
};

i32 main(i32 argc, char** argv)
{
	LayoutExampleWindow window;
	window.initialize(1280, 800, "OmniaFramework - Layout Example");
	window.setClearColor({ 0, 0, 0 });
	window.setPosition({ 50, 50 });
	window.mainLoop();
	return 0;
}
