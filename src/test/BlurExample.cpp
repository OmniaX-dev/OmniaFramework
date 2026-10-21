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
// Blur example — render targets + the built-in Gaussian blur, live.
//
//   - AnimatedScene: a full-window widget that renders a colorful, animated
//     scene into a RenderTarget every frame, then draws that target.
//   - FrostedPanel: a draggable Panel that draws the blurred part of the scene
//     right behind it ("frosted glass"), then its own (translucent) background
//     and its child widgets on top. Drag it around to see the blur follow.
//
// Controls (in the panel, or with keys):
//   blur radius slider  (Up / Down)     blur on/off           (B)
//   half-resolution scene target (H)    simulate device reset (R)
//   ESC quits. The status bar shows draw calls, cache stats and renderer info,
//   so batching regressions show up as a jump in the draw-call count.
//
// Note: the frosted glass blurs the scene layer (what AnimatedScene renders),
// not other widgets: widgets draw straight to the window, and reading the
// window back would be far too slow. Anything that should show through the
// glass belongs in the scene target.
// ============================================================================

#include <ogfx/ogfx.hpp>
#include <cmath>

using namespace ogfx;
using namespace ogfx::gui;

class AnimatedScene : public Widget
{
	public:
		inline AnimatedScene(Window& window) : Widget({ 0, 0, 0, 0 }, window)
		{
			setTypeName("AnimatedScene");
			disableFocus();
			m_target.setContentsLostCallback([this](RenderTarget&) { m_contentsLostCount++; });
			validate();
		}

		inline void applyTheme(const ostd::Stylesheet& theme) override
		{
			disableBackground();
			disableBorder();
		}

		inline void onUpdate(void) override
		{
			m_time += 1.0f / 60.0f;
		}

		inline void onDraw(BasicRenderer2D& gfx) override
		{
			const Vec2 size = getSize();
			if (size.x <= 0 || size.y <= 0)
				return;
			// The target follows the widget size (window resizes) and the resolution setting.
			const f32 s = m_resolutionScale;
			if (!m_target.isCreated())
				m_target.create(gfx, size * s);
			else
				m_target.resize(size * s);

			if (gfx.beginRenderTarget(m_target, true, Color(12, 12, 20, 255)))
			{
				draw_scene(gfx, size * s, s);
				gfx.endRenderTarget();
			}
			gfx.drawRenderTarget(m_target, getGlobalPosition(), size);
		}

		inline RenderTarget& getTarget(void) { return m_target; }
		inline f32 getResolutionScale(void) const { return m_resolutionScale; }
		inline void setResolutionScale(f32 scale) { m_resolutionScale = std::clamp(scale, 0.1f, 1.0f); }
		inline i32 getContentsLostCount(void) const { return m_contentsLostCount; }

	private:
		inline void draw_scene(BasicRenderer2D& gfx, const Vec2& size, f32 s)
		{
			const f32 t = m_time;
			ColorGradient bg({ Color(40, 10, 90), Color(10, 60, 120), Color(90, 20, 60) }, { 1.0f, 1.0f });
			bg.setAngleDeg(t * 10.0f);
			gfx.fillGradientRect({ -size.x * 0.25f, -size.y * 0.25f, size.x * 1.5f, size.y * 1.5f }, bg);

			// Vertical stripes sliding sideways: sharp edges make the blur easy to judge.
			for (i32 i = 0; i < 24; i++)
			{
				f32 x = std::fmod(i * 80.0f * s + t * 40.0f * s, size.x + 80.0f * s) - 40.0f * s;
				gfx.drawLine({ { x, 0 }, { x + 60.0f * s, size.y } }, Color(255, 255, 255, 18), cast<i32>(std::max(1.0f, 6.0f * s)), false);
			}

			// Orbiting circles.
			const Color palette[] = { Colors::Crimson, Colors::Orange, Colors::Gold, Colors::Lime, Colors::Cyan, Colors::RoyalBlue, Colors::Magenta, Colors::Pink };
			const Vec2 c { size.x * 0.5f, size.y * 0.5f };
			for (i32 i = 0; i < 16; i++)
			{
				f32 a = t * (0.3f + i * 0.04f) + i * 0.7f;
				f32 r = (120.0f + 40.0f * std::sin(t + i)) * s + (i % 4) * 70.0f * s;
				Vec2 p { c.x + std::cos(a) * r * 1.6f, c.y + std::sin(a) * r };
				Color col = palette[i % 8];
				col.a = 200;
				gfx.fillCircle(p, (24.0f + 10.0f * std::sin(t * 2.0f + i)) * s, col);
				gfx.drawCircle(p, (34.0f + 10.0f * std::sin(t * 2.0f + i)) * s, Color(255, 255, 255, 120), cast<i32>(std::max(1.0f, 2.0f * s)));
			}

			// Bouncing round rects and a spinning triangle.
			for (i32 i = 0; i < 5; i++)
			{
				f32 x = (0.1f + 0.18f * i) * size.x;
				f32 y = size.y * 0.75f + std::sin(t * 1.5f + i) * 60.0f * s;
				Color col = palette[(i * 3) % 8];
				col.a = 170;
				gfx.fillRoundRect({ x, y, 110.0f * s, 60.0f * s }, col, 16.0f * s);
				gfx.drawRoundRect({ x, y, 110.0f * s, 60.0f * s }, Colors::White, 16.0f * s, cast<i32>(std::max(1.0f, 2.0f * s)));
			}
			f32 a = t * 0.8f;
			Vec2 tc { size.x * 0.8f, size.y * 0.3f };
			f32 tr = 90.0f * s;
			gfx.fillTriangle({ tc.x + std::cos(a) * tr, tc.y + std::sin(a) * tr },
							 { tc.x + std::cos(a + 2.094f) * tr, tc.y + std::sin(a + 2.094f) * tr },
							 { tc.x + std::cos(a + 4.189f) * tr, tc.y + std::sin(a + 4.189f) * tr }, Color(255, 220, 60, 210));

			// Scrolling title.
			i32 fontSize = std::max(8, cast<i32>(72 * s));
			f32 tx = std::fmod(t * 120.0f * s, size.x + 900.0f * s) - 700.0f * s;
			gfx.drawString("OmniaFramework", { tx, size.y * 0.08f }, Colors::White, fontSize);
		}

	private:
		RenderTarget m_target;
		f32 m_time { 0.0f };
		f32 m_resolutionScale { 1.0f };
		i32 m_contentsLostCount { 0 };
};

class FrostedPanel : public Panel
{
	public:
		inline FrostedPanel(Window& window, AnimatedScene& scene) : Panel(window), m_scene(scene) { setTypeName("FrostedPanel"); }

		inline void applyTheme(const ostd::Stylesheet& theme) override
		{
			Panel::applyTheme(theme);
			// Translucent tint over the blur; square corners so the tint matches the
			// (rectangular) blurred region exactly.
			setBackgroundColor(Color(10, 10, 25, m_blurEnabled ? 70 : 150));
			setBorderColor(Color(255, 255, 255, 90));
			setBorderRadius(0);
			enableBorder();
			setTitlebarType(TitleBarTypes::Full);
			setTitle("Frosted glass (drag me)");
		}

		inline void beforeDraw(BasicRenderer2D& gfx) override
		{
			if (!m_blurEnabled)
				return;
			RenderTarget& target = m_scene.getTarget();
			if (!target.hasTexture())
				return;
			// The part of the scene right behind the panel, in the target's own units.
			const f32 s = m_scene.getResolutionScale();
			const Rectangle bounds = getGlobalBounds();
			const Vec2 scenePos = m_scene.getGlobalPosition();
			const Rectangle src { (bounds.x - scenePos.x) * s, (bounds.y - scenePos.y) * s, bounds.w * s, bounds.h * s };
			gfx.drawRenderTargetBlurred(target, bounds.getPosition(), m_radius * s, bounds.getSize(), Colors::White, src);
		}

		inline void setBlurEnabled(bool enabled) { m_blurEnabled = enabled; setBackgroundColor(Color(10, 10, 25, enabled ? 70 : 150)); }
		inline bool isBlurEnabled(void) const { return m_blurEnabled; }
		inline void setRadius(f32 radius) { m_radius = std::max(0.0f, radius); }
		inline f32 getRadius(void) const { return m_radius; }

	private:
		AnimatedScene& m_scene;
		bool m_blurEnabled { true };
		f32 m_radius { 16.0f };
};

class BlurExampleWindow : public Window
{
	public:
		inline BlurExampleWindow(void) {  }

		inline void onInitialize(void) override
		{
			m_theme.loadFromFile("./DefaultThemeDark.oss", true, getDefaultStylesheetVariableList());
			setTheme(m_theme);

			addWidget(m_scene);
			m_scene.setAnchorPreset(eAnchorPreset::FullRect);

			build_panel();
			build_status_bar();
			update_radius_label();
		}

		inline void onSignal(ostd::Signal& signal) override
		{
			if (signal.ID != ostd::BuiltinSignals::KeyReleased)
				return;
			auto& evtData = *(ogfx::KeyEventData*)signal.userData;
			switch (evtData.keyCode)
			{
				case ogfx::KeyCode::Escape: close(); break;
				case ogfx::KeyCode::B: m_blurCheck.setChecked(!m_blurCheck.isChecked()); break;
				case ogfx::KeyCode::H: toggle_half_resolution(); break;
				case ogfx::KeyCode::R: getGFX().simulateDeviceReset(); break;
				case ogfx::KeyCode::Up: set_radius(m_panel.getRadius() + 2.0f); break;
				case ogfx::KeyCode::Down: set_radius(m_panel.getRadius() - 2.0f); break;
				default: break;
			}
		}

		inline void onUpdate(f64 delta) override
		{
			// Refreshing the labels a few times per second is plenty (and keeps layout work low).
			m_statsTimer += delta;
			if (m_statsTimer < 0.25)
				return;
			m_statsTimer = 0.0;
			auto& gfx = getGFX();
			const IPoint px = m_scene.getTarget().getPixelSize();
			m_statsLbl.setText(String("Draw calls: ").add((i32)gfx.getDrawCallCount())
				.add("  |  Cache hit/miss: ").add((i32)gfx.getCacheHitCount()).add("/").add((i32)gfx.getCacheMissCount())
				.add("  |  Target: ").add(px.x).add("x").add(px.y)
				.add("  |  Lost: ").add(m_scene.getContentsLostCount()));
			m_rendererLbl.setText(String(SDL_GetRendererName(getSDLRenderer())).add(gfx.areShadersSupported() ? " + shaders" : " (no shaders)"));
		}

	private:
		inline void build_panel(void)
		{
			addWidget(m_panel);
			m_panel.enableDraggable();
			m_panel.setBounds({ 60, 60, 420, 440 });

			m_panel.addWidget(m_box);
			m_box.setAnchorPreset(eAnchorPreset::FullRect, ePresetResizeMode::KeepSize, 12);
			m_box.setSpacing(8);

			m_info.setText("The scene behind is blurred live.");
			m_box.addWidget(m_info);

			m_radiusLbl.setText("Radius");
			m_box.addWidget(m_radiusLbl);
			m_radiusSlider.setValueQuiet(m_panel.getRadius());
			m_radiusSlider.setValueChangedCallback([this](f32, f32 value) {
				m_panel.setRadius(value);
				update_radius_label();
			});
			m_box.addWidget(m_radiusSlider);

			m_blurCheck.setText("Blur enabled (B)");
			m_blurCheck.setChecked(true);
			m_blurCheck.setStateChangedCallback([this](CheckBox&, bool checked) { m_panel.setBlurEnabled(checked); });
			m_box.addWidget(m_blurCheck);

			m_halfResBtn.setText("Half-resolution scene target (H)");
			m_halfResBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { toggle_half_resolution(); });
			m_box.addWidget(m_halfResBtn);

			m_resetBtn.setText("Simulate device reset (R)");
			m_resetBtn.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { getGFX().simulateDeviceReset(); });
			m_box.addWidget(m_resetBtn);

			m_progress.enableShowText();
			m_box.addWidget(m_progress);
		}

		inline void build_status_bar(void)
		{
			getStatusBar().setHeight(28);
			showStatusBar(true);
			getStatusBar().addWidget(m_statsLbl);
			m_statsLbl.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 6);
			getStatusBar().addWidget(m_rendererLbl);
			m_rendererLbl.setAnchorPreset(eAnchorPreset::CenterRight, ePresetResizeMode::KeepSize, 8);
		}

		inline void toggle_half_resolution(void)
		{
			m_scene.setResolutionScale(m_scene.getResolutionScale() < 1.0f ? 1.0f : 0.5f);
			m_halfResBtn.setText(m_scene.getResolutionScale() < 1.0f ? "Full-resolution scene target (H)" : "Half-resolution scene target (H)");
		}

		// Keys change the radius programmatically: setValueQuiet() doesn't run the slider
		// callback, so the panel is updated here as well.
		inline void set_radius(f32 radius)
		{
			m_radiusSlider.setValueQuiet(radius);
			m_panel.setRadius(m_radiusSlider.getValue());
			update_radius_label();
		}

		inline void update_radius_label(void)
		{
			m_radiusLbl.setText(String("Radius: ").add((i32)std::round(m_panel.getRadius())).add(" (Up / Down)"));
			m_progress.setProgressNormalized(m_panel.getRadius() / 48.0f);
		}

	private:
		ostd::Stylesheet m_theme;
		AnimatedScene m_scene { *this };
		FrostedPanel m_panel { *this, m_scene };
		VBoxContainer m_box { *this };
		Label m_info { *this };
		Label m_radiusLbl { *this };
		Slider m_radiusSlider { *this, false, 0.0f, 48.0f, 1.0f };
		CheckBox m_blurCheck { *this };
		Button m_halfResBtn { *this };
		Button m_resetBtn { *this };
		ProgressBar m_progress { *this };
		Label m_statsLbl { *this };
		Label m_rendererLbl { *this };
		f64 m_statsTimer { 1.0 };
};

i32 main(i32 argc, char** argv)
{
	BlurExampleWindow window;
	window.initialize(1280, 800, "OmniaFramework - Blur Example");
	window.setClearColor({ 0, 0, 0 });
	window.mainLoop();
	return 0;
}
