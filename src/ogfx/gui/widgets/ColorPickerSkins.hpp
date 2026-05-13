/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev
	(license header same as the project)
*/

#pragma once

#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/widgets/Slider.hpp>
#include <ogfx/gui/widgets/Label.hpp>
#include <ogfx/gui/widgets/Text.hpp>
#include <ogfx/gui/widgets/Button.hpp>
#include <ogfx/gui/widgets/ColorPicker.hpp>

namespace ogfx
{
	namespace gui
	{
		// ===========================================================================
		// ColorPickerPanel — slider-driven RGBA picker.
		//
		// An alternate "skin" for ColorPicker: four sliders (R, G, B, A) plus a
		// swatch and hex field. Useful when the user wants precise numeric input
		// rather than dragging a square. Internally it owns a ColorPicker as the
		// authoritative state holder (so the two forms can be cross-bound) but
		// you can also use it standalone — calls to setColor/getColor just go
		// straight to the underlying picker.
		//
		// The constructor takes the parent ColorPicker by reference so you can
		// share state across multiple skins of the same color. If you want
		// standalone behavior, pass an internally-owned picker (see the
		// `createStandalone` factory below).
		// ===========================================================================
		class ColorPickerPanel : public Widget
		{
			public:
				inline ColorPickerPanel(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				ColorPickerPanel& create(void);

				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;

				Color getColor(void) const;
				void setColor(const Color& c);

				inline void setColorChangedCallback(std::function<void(const Color&)> cb)
				{ callback_onColorChanged = std::move(cb); }

			private:
				void on_slider_changed(void);
				void sync_sliders_from_color(const Color& c);
				void layout_children(void);

			private:
				Slider* m_rSlider { nullptr };
				Slider* m_gSlider { nullptr };
				Slider* m_bSlider { nullptr };
				Slider* m_aSlider { nullptr };
				Label*  m_rLabel { nullptr };
				Label*  m_gLabel { nullptr };
				Label*  m_bLabel { nullptr };
				Label*  m_aLabel { nullptr };
				TextEdit* m_hexField { nullptr };

				Color m_currentColor { Colors::White };
				bool m_suppressCallbacks { false };

				std::function<void(const Color&)> callback_onColorChanged { nullptr };
		};


		// ===========================================================================
		// ColorButton — small clickable swatch that pops up a ColorPicker.
		//
		// Behaves like a button: shows the current color (over a checkerboard so
		// alpha is visible), and clicking it opens a floating ColorPicker just
		// below the swatch. The popup is a TopMost root widget added to the
		// window; it closes when the user clicks outside it.
		// ===========================================================================
		class ColorButton : public Widget
		{
			public:
				inline ColorButton(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				ColorButton& create(void);

				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMouseReleased(const Event& event) override;
				void onUpdate(void) override;

				inline Color getColor(void) const { return m_color; }
				void setColor(const Color& c);

				inline void setColorChangedCallback(std::function<void(const Color&)> cb)
				{ callback_onColorChanged = std::move(cb); }

				OSTD_PARAM_GETSET(f32, CheckerSize, m_checkerSize);

			private:
				void open_popup(void);
				void close_popup(void);
				void draw_checkerboard(BasicRenderer2D& gfx, const Rectangle& r);

			private:
				Color m_color { Colors::White };
				f32 m_checkerSize { 4.0f };

				// The popup is a separate root-level widget owned by us.
				// We allocate it on first open and destroy it on close to keep
				// the widget tree clean.
				ColorPicker* m_popup { nullptr };
				bool m_popupOpen { false };

				std::function<void(const Color&)> callback_onColorChanged { nullptr };
		};
	}
}
