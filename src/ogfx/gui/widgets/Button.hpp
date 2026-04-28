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
#include <ogfx/resources/Image.hpp>
#include <ogfx/utils/Animation.hpp>

namespace ogfx
{
	namespace gui
	{
		// ===========================================================================
		// eSizeMode — shared across Button / Label / (and analogous on ImageLabel).
		//
		//   AutoSize:      widget computes its own size to fit content. Used when
		//                  there's no layout managing it, and the user wants the
		//                  widget to "just be the right size".
		//
		//   Manual:        user controls the widget's size; user also controls the
		//                  content sizes (text/icon). Nothing is computed automatically.
		//                  Content is drawn at its user-specified size, centered, and
		//                  clipped at the widget edge if it doesn't fit.
		//
		//   LayoutManaged: parent's Layout controls the widget's size. Content
		//                  (text/icon) is at its user-specified size, and the widget
		//                  honors whatever bounds the layout assigns.
		//                  This mode is auto-selected when added to a parent that
		//                  has a layout — unless the user explicitly set Manual.
		// ===========================================================================
		enum class eSizeMode : u8
		{
			AutoSize,
			Manual,
			LayoutManaged
		};

		class Button : public Widget
		{
			public:
				inline Button(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
				inline Button(WindowCore& window, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
				Button& create(const String& text);

				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;
				void onMouseReleased(const Event& event) override;
				void onBoundsChanged(const Vec2& newSize) override;
				void applyTheme(const ostd::Stylesheet& theme) override;

				void setText(const String& text);
				void setIcon(const String& filePath);
				inline String getText(void) const { return m_text; }
				inline Image& getIcon(void) { return m_icon; }

				// =========================== SIZE MODE ============================
				// Set the widget's size mode explicitly. After calling this, the
				// auto-detection (LayoutManaged when parent has a layout) is
				// disabled — the user's choice is respected.
				void setSizeMode(eSizeMode mode);
				inline eSizeMode getSizeMode(void) const { return m_sizeMode; }

				// The mode actually used by the layout/draw code. Resolves auto-
				// detection: if the user hasn't explicitly set a mode and the
				// parent has a layout, this returns LayoutManaged.
				eSizeMode effectiveSizeMode(void) const;
				// ==================================================================

				OSTD_BOOL_PARAM_GETSET_E(IconOnly, m_iconOnly);
				OSTD_BOOL_PARAM_GETSET_E(Animated, m_animated);
				OSTD_BOOL_PARAM_GETSET_E(Icon, m_showIcon);
				OSTD_PARAM_GETSET(ostd::AnimationData, AnimationData, m_animData);
				OSTD_PARAM_GETSET(Vec2, IconSize, m_iconSize);
				OSTD_PARAM_GETSET(f32, IconSpacing, m_iconSpacing);
				OSTD_PARAM_GETSET(Color, IconTintColor, m_iconTint);

			private:
				inline void invalidate_layout(void) { m_layoutDirty = true; }

				// Re-runs the size + content computation. What this does depends
				// entirely on effectiveSizeMode() — see the cpp for details.
				void recompute_layout(void);

				// Returns the icon size to draw, given the current effective text
				// height. In AutoSize: max(text-height, hintFloor) with aspect
				// preserved. In Manual/LayoutManaged: the user's m_iconSize hint
				// (or fontSize fallback). Aspect ratio is always preserved when
				// the icon is loaded with a known natural size.
				Vec2 compute_icon_size(f32 textHeightHint) const;

				// Native floor for the icon: m_iconSize if set, else font size.
				// (User-configurable hint as discussed; the icon will not be
				// drawn smaller than this in AutoSize mode.)
				Vec2 icon_floor(void) const;

			private:
				String m_text { "" };
				Image m_icon;
				ostd::AnimationData m_animData;
				Animation m_anim;
				Vec2 m_iconSize { 0, 0 };
				f32 m_iconSpacing { 10 };
				Color m_iconTint { Colors::White };

				// ===== Mode =====
				eSizeMode m_sizeMode { eSizeMode::AutoSize };
				bool m_modeExplicitlySet { false };
				// ================

				// ===== Computed draw state (refreshed by recompute_layout) =====
				bool m_layoutDirty { true };
				Vec2 m_drawIconSize { 0, 0 };
				Vec2 m_drawIconOffset { 0, 0 };
				Vec2 m_drawTextOffset { 0, 0 };
				// ===============================================================

				// ===== Flags =====
				bool m_iconOnly { false };
				bool m_showIcon { false };
				bool m_animated { false };
				// =================
		};
	}
}
