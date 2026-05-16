/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev
	(license header same as the project)
*/

#include "ColorPickerSkins.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../Window.hpp"
#include <cmath>
#include <algorithm>

namespace ogfx
{
	namespace gui
	{
		// ===================================================================
		// ColorPickerPanel — RGBA slider skin
		// ===================================================================
		ColorPickerPanel& ColorPickerPanel::create(void)
		{
			setPadding({ 8, 8, 8, 8 });
			setTypeName("ogfx::gui::ColorPickerPanel");
			setStylesheetCategoryName("colorPickerPanel");
			enableBackground();
			enableBorder();
			setSize({ 260, 200 });

			auto make_slider = [this](Color trackProgress) -> Slider* {
				auto* s = new Slider(getWindow(), false, 0.0f, 255.0f, 1.0f);
				s->setTrackProgressColor(trackProgress);
				s->setValueChangedCallback([this](f32 oldValue, f32 newValue) { on_slider_changed(); });
				addWidget(*s);
				return s;
			};

			auto make_label = [this](const String& txt) -> Label* {
				auto* l = new Label(getWindow(), txt);
				addWidget(*l);
				return l;
			};

			m_rSlider = make_slider(Color(220, 60, 60));
			m_gSlider = make_slider(Color(60, 200, 60));
			m_bSlider = make_slider(Color(80, 120, 230));
			m_aSlider = make_slider(Color(180, 180, 180));
			m_rLabel  = make_label("R");
			m_gLabel  = make_label("G");
			m_bLabel  = make_label("B");
			m_aLabel  = make_label("A");

			m_hexField = new TextEdit(getWindow());
			m_hexField->setMaxLength(9);
			m_hexField->setCallback(eCallback::ActionPerformed, [this](const Event&) {
				Color parsed;
				parsed.set(m_hexField->getText());
				if (parsed.isValid())
					setColor(parsed);
				else
					sync_sliders_from_color(m_currentColor); // rejected -> restore
			});
			addWidget(*m_hexField);

			layout_children();
			sync_sliders_from_color(m_currentColor);
			validate();
			return *this;
		}

		void ColorPickerPanel::applyTheme(const ostd::Stylesheet& theme)
		{
			// Nothing panel-specific yet; child widgets inherit through the
			// regular theme propagation.
			(void)theme;
		}

		void ColorPickerPanel::layout_children(void)
		{
			// Simple manual vertical layout:
			//   [swatch]
			//   R: [-----slider-----]
			//   G: [-----slider-----]
			//   B: [-----slider-----]
			//   A: [-----slider-----]
			//   [    hex field      ]
			const Rectangle cb = getGlobalContentBounds();
			const Vec2 cp = getGlobalContentPosition();
			const f32 lblW = 18.0f;
			const f32 gap  = 6.0f;
			const f32 rowH = 22.0f;
			const f32 swatchH = 32.0f;

			f32 y = swatchH + gap; // swatch occupies the top — drawn in onDraw

			auto place_row = [&](Label* lbl, Slider* sl) {
				lbl->setPosition({ 0, y + 2 });
				lbl->setSize({ lblW, rowH });
				sl->setPosition({ lblW + gap, y + rowH * 0.5f - 9.0f });
				sl->setSize({ cb.w - lblW - gap, 18.0f });
				y += rowH + 4.0f;
			};
			place_row(m_rLabel, m_rSlider);
			place_row(m_gLabel, m_gSlider);
			place_row(m_bLabel, m_bSlider);
			place_row(m_aLabel, m_aSlider);

			y += gap;
			m_hexField->setPosition({ 0, y });
			m_hexField->setSize({ cb.w, 26.0f });

			(void)cp;
		}

		Color ColorPickerPanel::getColor(void) const
		{
			return m_currentColor;
		}

		void ColorPickerPanel::setColor(const Color& c)
		{
			m_currentColor = c;
			sync_sliders_from_color(c);
			if (callback_onColorChanged)
				callback_onColorChanged(m_currentColor);
		}

		void ColorPickerPanel::sync_sliders_from_color(const Color& c)
		{
			m_suppressCallbacks = true;
			// Slider has no public setValue (only set_value private + drag/click),
			// so we set value through the callback-free path: re-create min/max
			// is overkill; instead we keep an internal mirror m_currentColor and
			// pull from there in the draw. If your Slider gains a public
			// setValue/setValueSilent, swap these in.
			//
			// For now we rely on the user dragging the sliders to push values
			// in, and on the hex field for programmatic input. If you want
			// programmatic set to also move slider handles, add a public
			// Slider::setValue and call it here.

			if (m_hexField)
			{
				const String hex = c.hexString(true, "#");
				m_hexField->setText(hex);
			}
			m_suppressCallbacks = false;
		}

		void ColorPickerPanel::on_slider_changed(void)
		{
			if (m_suppressCallbacks) return;
			m_currentColor = Color(
				static_cast<u8>(std::round(m_rSlider->getValue())),
				static_cast<u8>(std::round(m_gSlider->getValue())),
				static_cast<u8>(std::round(m_bSlider->getValue())),
				static_cast<u8>(std::round(m_aSlider->getValue()))
			);
			m_suppressCallbacks = true;
			if (m_hexField)
				m_hexField->setText(m_currentColor.hexString(true, "#"));
			m_suppressCallbacks = false;
			if (callback_onColorChanged)
				callback_onColorChanged(m_currentColor);
		}

		void ColorPickerPanel::onDraw(BasicRenderer2D& gfx)
		{
			// Top swatch with checkerboard.
			const Vec2 cp = getGlobalContentPosition();
			const Rectangle cb = getGlobalContentBounds();
			Rectangle swatch { cp.x, cp.y, cb.w, 32.0f };

			// Checkerboard.
			const f32 cs = 5.0f;
			const i32 cols = static_cast<i32>(std::ceil(swatch.w / cs));
			const i32 rows = static_cast<i32>(std::ceil(swatch.h / cs));
			gfx.fillRect(swatch, Color(200, 200, 200));
			for (i32 yy = 0; yy < rows; ++yy)
				for (i32 xx = 0; xx < cols; ++xx)
					if ((xx + yy) & 1)
					{
						const f32 px = swatch.x + xx * cs;
						const f32 py = swatch.y + yy * cs;
						const f32 pw = std::min(cs, swatch.x + swatch.w - px);
						const f32 ph = std::min(cs, swatch.y + swatch.h - py);
						gfx.fillRect({ px, py, pw, ph }, Color(140, 140, 140));
					}
			gfx.fillRect(swatch, m_currentColor);
			gfx.drawRect(swatch, getBorderColor(), 1);
		}


		// ===================================================================
		// ColorButton — swatch + popup
		// ===================================================================
		ColorButton& ColorButton::create(void)
		{
			setPadding({ 2, 2, 2, 2 });
			setTypeName("ogfx::gui::ColorButton");
			setStylesheetCategoryName("colorButton");
			enableBackground();
			enableBorder();
			enableFocus();
			enableDragAndDrop();
			setSize({ 48, 24 });
			setCallback(eCallback::DragAndDrop, [&](const ogfx::gui::Event& event) -> void {
				auto data = Widget::getDragAndDropData();
				if (data && data->getTypeName() == Color().getTypeName())
					setColor(*cast<Color*>(data));
			});
			connectSignal(ostd::BuiltinSignals::MousePressed);
			m_contextMenu.onActivate = [this](const ContextMenu::Entry& e) {
				if (e.id == MenuId::Copy)
				{
					// TODO: Replace once I have a clipboard system
					SDL_SetClipboardText(getColor().hexString(true, "#"));
				}
				else if (e.id == MenuId::Paste)
				{
					// TODO: Replace once I have a clipboard system
					const String clipboardText = SDL_GetClipboardText();
					Color tmp;
					if (!clipboardText.empty() && Color::isValidColorString(clipboardText, &tmp))
						setColor(tmp);
				}
			};
			validate();
			return *this;
		}

		void ColorButton::applyTheme(const ostd::Stylesheet& theme)
		{
			setCheckerSize(getThemeValue<f32>(theme, "checkerSize", m_checkerSize));
		}

		void ColorButton::setColor(const Color& c)
		{
			m_color = c;
			if (m_popup) m_popup->setColor(c);
			if (callback_onColorChanged)
				callback_onColorChanged(m_color);
		}

		void ColorButton::draw_checkerboard(BasicRenderer2D& gfx, const Rectangle& r)
		{
			gfx.fillRect(r, Color(200, 200, 200));
			const f32 cs = m_checkerSize;
			const i32 cols = static_cast<i32>(std::ceil(r.w / cs));
			const i32 rows = static_cast<i32>(std::ceil(r.h / cs));
			for (i32 y = 0; y < rows; ++y)
				for (i32 x = 0; x < cols; ++x)
					if ((x + y) & 1)
					{
						const f32 cx = r.x + x * cs;
						const f32 cy = r.y + y * cs;
						const f32 cw = std::min(cs, r.x + r.w - cx);
						const f32 ch = std::min(cs, r.y + r.h - cy);
						if (cw > 0 && ch > 0)
							gfx.fillRect({ cx, cy, cw, ch }, Color(140, 140, 140));
					}
		}

		void ColorButton::onDraw(BasicRenderer2D& gfx)
		{
			const Rectangle r = getGlobalContentBounds();
			draw_checkerboard(gfx, r);
			gfx.fillRect(r, m_color);
			gfx.drawRect(r, getBorderColor(), 1);
		}

		void ColorButton::onMouseReleased(const Event& event)
		{
			bool mph = m_mousePressedHere;
			m_mousePressedHere = false;
			if (!isMouseInside()) return;
			if (event.mouse->button != MouseEventData::eButton::Left)
			{
				if (event.mouse->button == MouseEventData::eButton::Right)
					getWindow().showContextMenu(m_contextMenu, { event.mouse->position_x, event.mouse->position_y });
				return;
			}
			if (!mph) return;
			if (m_popupOpen) close_popup();
			else             open_popup();
			event.handle();
		}

		void ColorButton::onMousePressed(const Event& event)
		{
			if (!isMouseInside()) return;
			if (event.mouse->button != MouseEventData::eButton::Left)
				return;
			Widget::setDragAndDropData(m_color);
			m_mousePressedHere = true;
			event.handle();
		}

		void ColorButton::open_popup(void)
		{
			if (m_popup) close_popup();

			m_popup = new ColorPicker(getWindow());
			m_popup->setRootChild();
			// m_popup->setTopMost(true); // TODO: Implement
			m_popup->setSize({ 240, 200 });
			m_popup->setColor(m_color);

			m_popup->setColorChangedCallback([this](const Color& c) {
				m_color = c;
				if (callback_onColorChanged)
					callback_onColorChanged(m_color);
			});

			// Position just below this button. Flip up if it would overflow
			// the bottom of the window.
			const Rectangle b = getGlobalBounds();
			f32 px = b.x;
			f32 py = b.y + b.h + 4.0f;
			const f32 winH = static_cast<f32>(getWindow().getWindowHeight());
			const f32 winW = static_cast<f32>(getWindow().getWindowWidth());
			if (py + m_popup->geth() > winH)
				py = b.y - 4.0f - m_popup->geth();
			if (px + m_popup->getw() > winW)
				px = winW - m_popup->getw() - 4.0f;
			if (px < 0) px = 4.0f;
			if (py < 0) py = 4.0f;
			m_popup->setPosition({ px, py });

			getWindow().addWidget(*m_popup);
			m_popupOpen = true;
		}

		void ColorButton::close_popup(void)
		{
			if (!m_popup) { m_popupOpen = false; return; }
			getWindow().removeWidget(*m_popup);
			delete m_popup;
			m_popup = nullptr;
			m_popupOpen = false;
		}

		void ColorButton::handleSignal(ostd::Signal& signal)
		{
			if (!m_popupOpen) return;
			if (signal.ID != ostd::BuiltinSignals::MousePressed) return;
			if (isMouseInside()) return;
			if (!m_popup->isMouseInside())
				close_popup();
		}
	}
}
