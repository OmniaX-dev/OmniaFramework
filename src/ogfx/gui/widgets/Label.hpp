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

#pragma once

#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/widgets/Button.hpp>   // for eSizeMode
#include <ogfx/resources/Image.hpp>
#include <ogfx/utils/Animation.hpp>

namespace ogfx
{
	namespace gui
	{
		// ===========================================================================
		// Label — text-only widget.
		//
		// Modes (using the shared eSizeMode):
		//   AutoSize:      widget shrinks/grows to fit text + padding.
		//   Manual:        widget keeps user-set size; text drawn inside, clipped
		//                  if it doesn't fit.
		//   LayoutManaged: parent layout decides size; otherwise behaves like Manual.
		// ===========================================================================
		class Label : public Widget
		{
			public:
				inline Label(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
				inline Label(WindowCore& window, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
				Label& create(const String& text);

				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onBoundsChanged(const Vec2& newSize) override;

				void setText(const String& text);
				inline String getText(void) const { return m_text; }

				// Same semantics as Button::setSizeMode / effectiveSizeMode.
				void setSizeMode(eSizeMode mode);
				inline eSizeMode getSizeMode(void) const { return m_sizeMode; }
				eSizeMode effectiveSizeMode(void) const;

			private:
				inline void invalidate_layout(void) { m_layoutDirty = true; }
				void recompute_layout(void);

			private:
				String m_text { "" };
				eSizeMode m_sizeMode { eSizeMode::AutoSize };
				bool m_modeExplicitlySet { false };
				bool m_layoutDirty { true };
				Vec2 m_drawTextOffset { 0, 0 };
		};


		// ===========================================================================
		// ImageLabel — image-only widget.
		//
		// Two image modes (separate from widget size — the widget's size always
		// comes from the user or from the parent layout):
		//
		//   eImageFit::Fit:      image scales to fill the widget's content area.
		//                        Optional keepAspectRatio: if true, image is
		//                        scaled uniformly to fit and centered (letterbox);
		//                        if false, image stretches non-uniformly to fill.
		//
		//   eImageFit::Native:   image drawn at user-specified m_imageSize,
		//                        centered inside the widget bounds. Crops naturally
		//                        if m_imageSize > widget content size — the widget
		//                        clips at its own bounds anyway.
		// ===========================================================================
		enum class eImageFit : u8
		{
			Fit,
			Native
		};

		class ImageLabel : public Widget
		{
			public:
				inline ImageLabel(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
				inline ImageLabel(WindowCore& window, const String& filePath) : Widget({ 0, 0, 0, 0 }, window) { create(filePath); }
				ImageLabel& create(const String& filePath);

				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;

				void setImage(const String& filePath);
				inline Image& getImage(void) { return m_image; }

				// Mode selection. Default is Fit + keepAspectRatio=true.
				inline void setImageFit(eImageFit fit) { m_fit = fit; }
				inline eImageFit getImageFit(void) const { return m_fit; }

				// Only meaningful when fit == Fit. If true, image keeps its
				// aspect ratio and is centered with letterbox bars; if false,
				// image stretches non-uniformly to fill.
				OSTD_BOOL_PARAM_GETSET_E(KeepAspectRatio, m_keepAspect);

				// Only meaningful when fit == Native. The size to draw the image at,
				// regardless of widget size. The image is drawn centered.
				OSTD_PARAM_GETSET(Vec2, ImageSize, m_imageSize);

				OSTD_BOOL_PARAM_GETSET_E(Animated, m_animated);
				OSTD_PARAM_GETSET(AnimationData, AnimationData, m_animData);
				OSTD_PARAM_GETSET(Color, TintColor, m_tintColor);

			private:
				// Compute (drawPosition, drawSize) for the current frame.
				// drawPosition is in global coords. drawSize is the on-screen size.
				void compute_draw_geometry(Vec2& outPos, Vec2& outSize) const;

			private:
				Image m_image;
				Animation m_anim;
				AnimationData m_animData;
				Color m_tintColor { Colors::White };

				eImageFit m_fit { eImageFit::Fit };
				bool m_keepAspect { true };
				Vec2 m_imageSize { 64, 64 };  // user-specified size for Native mode

				bool m_animated { false };
		};
	}
}
