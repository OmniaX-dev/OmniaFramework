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
		namespace widgets
		{
			class Button : public Widget
			{
				public:
					inline Button(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
					inline Button(WindowCore& window, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					Button& create(const String& text);
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void onUpdate(void) override;
					void onMouseReleased(const Event& event) override;
					void setText(const String& text);
					void setIcon(const String& filePath);
					void applyTheme(const ostd::Stylesheet& theme) override;
					inline String getText(void) const { return m_text; }
					inline Image& getIcon(void) { return m_icon; }

					// When auto-size is enabled, the widget resizes itself to fit text+icon+padding
					// at native font/icon size (text is drawn at scale 1.0).
					// When auto-size is disabled, the widget keeps its user-defined size, and the
					// content is scaled uniformly down/up to fit that size (text uses the renderer's
					// scale parameter; icon is scaled proportionally).
					OSTD_BOOL_PARAM_GETSET_E(AutoSize, m_autoSize);

					// Icon-only mode: text is ignored when laying out and drawing.
					// Combined with auto-size, the button becomes a square based on font size.
					// Combined with manual size, the icon scales to fit the content area.
					OSTD_BOOL_PARAM_GETSET_E(IconOnly, m_iconOnly);

					OSTD_BOOL_PARAM_GETSET_E(Animated, m_animated);
					OSTD_BOOL_PARAM_GETSET_E(Icon, m_showIcon);
					OSTD_PARAM_GETSET(ostd::AnimationData, AnimationData, m_animData);
					OSTD_PARAM_GETSET(Vec2, IconSize, m_iconSize);
					OSTD_PARAM_GETSET(f32, IconSpacing, m_iconSpacing);
					OSTD_PARAM_GETSET(Color, IconTintColor, m_iconTint);

				private:
					// Force a layout recompute on the next draw.
					inline void invalidate_layout(void) { m_layoutDirty = true; }

					// Recomputes layout. In auto-size mode this also resizes the widget.
					// In manual-size mode this computes the scale factor that fits content
					// into the current widget size.
					void recompute_layout(void);

					// Returns the icon size at native (un-scaled) resolution. If m_iconSize is
					// (0,0) and an icon is loaded, falls back to the icon's natural size; if no
					// icon is loaded, falls back to font-size-based square. Aspect ratio of
					// m_iconSize is preserved.
					Vec2 native_icon_size(void) const;

				private:
					String m_text { "" };
					Image m_icon;
					ostd::AnimationData m_animData;
					Animation m_anim;
					Vec2 m_iconSize { 0, 0 };
					f32 m_iconSpacing { 10 };
					Color m_iconTint { Colors::White };

					// Computed layout state (refreshed by recompute_layout).
					bool m_layoutDirty { true };
					f32 m_contentScale { 1.0f };       // applied to text & icon when !m_autoSize
					Vec2 m_drawIconSize { 0, 0 };      // final on-screen icon size
					Vec2 m_drawIconOffset { 0, 0 };    // top-left of the icon inside the content area
					Vec2 m_drawTextOffset { 0, 0 };    // top-left of the text inside the content area

					// Flags
					bool m_autoSize { true };
					bool m_iconOnly { false };
					bool m_showIcon { false };
					bool m_animated { false };
			};
		}
	}
}
