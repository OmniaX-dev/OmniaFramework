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

#include "Button.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../../../ostd/io/FileSystem.hpp"
#include "../Window.hpp"
#include <algorithm>

namespace ogfx
{
	namespace gui
	{
		Button& Button::create(const String& text)
		{
			m_text = text;
			setPadding({ 5, 5, 5, 5 });
			setTypeName("ogfx::gui::Button");
			disableChildren();
			enableBackground(false);
			setStylesheetCategoryName("button");
			validate();
			invalidate_layout();
			return *this;
		}

		// ===================================================================
		// Mode handling
		// ===================================================================
		void Button::setSizeMode(eSizeMode mode)
		{
			m_sizeMode = mode;
			m_modeExplicitlySet = true;
			invalidate_layout();
		}

		eSizeMode Button::effectiveSizeMode(void) const
		{
			// Auto-detect: if user never set a mode explicitly and our parent
			// owns a layout, we run as LayoutManaged. Otherwise honor the
			// stored mode (AutoSize by default).
			if (!m_modeExplicitlySet && getParent() && getParent()->hasLayout())
				return eSizeMode::LayoutManaged;
			return m_sizeMode;
		}

		// ===================================================================
		// Lifecycle hooks
		// ===================================================================
		void Button::onBoundsChanged(const Vec2& /*newSize*/)
		{
			// Layouts call this after they assign new bounds. Recompute the
			// content placement so it stays centered inside the new size.
			invalidate_layout();
		}

		void Button::onMouseReleased(const Event& event)
		{
			if (!isMouseInside())
				return;
			if (event.mouse->button == MouseEventData::eButton::Left && callback_onActionPerformed)
				callback_onActionPerformed(event);
		}

		void Button::onUpdate(void)
		{
			if (isAnimatedEnabled())
				m_anim.update();
		}

		void Button::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			if (m_layoutDirty)
				recompute_layout();

			const Vec2 contentTL = getGlobalContentPosition();

			// --- Icon ---
			if (isIconEnabled() && m_drawIconSize.x > 0 && m_drawIconSize.y > 0)
			{
				const Vec2 iconPos = contentTL + m_drawIconOffset;
				if (isAnimatedEnabled())
					gfx.drawAnimation(m_anim, iconPos, m_drawIconSize, getIconTintColor());
				else
					gfx.drawImage(m_icon, iconPos, m_drawIconSize, getIconTintColor());
			}

			// --- Text ---
			if (!isIconOnlyEnabled() && m_text.len() > 0)
			{
				gfx.drawString(
					m_text,
					contentTL + m_drawTextOffset,
					getTextColor(),
					getFontSize()
				);
			}
		}

		// ===================================================================
		// Setters that trigger a recompute
		// ===================================================================
		void Button::setText(const String& text)
		{
			m_text = text;
			invalidate_layout();
		}

		void Button::setIcon(const String& filePath)
		{
			disableIcon();
			if (filePath == "") { invalidate_layout(); return; }
			if (!ostd::FileSystem::fileExists(filePath)) { invalidate_layout(); return; }
			m_icon.destroy();
			m_icon.loadFromFile(filePath, getWindow().getGFX());
			enableIcon();
			invalidate_layout();
		}

		void Button::applyTheme(const ostd::Stylesheet& theme)
		{
			enableIcon(getThemeValue<bool>(theme, "showIcon", m_showIcon));
			enableIconOnly(getThemeValue<bool>(theme, "iconOnly", m_iconOnly));
			enableAnimated(getThemeValue<bool>(theme, "icon.animated", m_animated));
			setAnimationData(getThemeValue<AnimationData>(theme, "icon.animation", m_animData));
			setIconTintColor(getThemeValue<Color>(theme, "icon.tint", getIconTintColor()));
			String filePath = getThemeValue<String>(theme, "icon.path", m_icon.getFilePath());
			if (filePath != m_icon.getFilePath())
				setIcon(filePath);
			setIconSize(getThemeValue<Vec2>(theme, "icon.size", getIconSize()));
			setIconSpacing(getThemeValue<f32>(theme, "icon.spacing", m_iconSpacing));
			if (isAnimatedEnabled())
			{
				m_anim.create(m_animData);
				m_anim.setSpriteSheet(m_icon);
			}
			invalidate_layout();
		}

		// ===================================================================
		// Icon size helpers
		// ===================================================================
		Vec2 Button::icon_floor(void) const
		{
			// Configurable hint floor: if the user set m_iconSize, use it as the
			// minimum icon dimensions. Otherwise fall back to font size square.
			const f32 fontEdge = (f32)getFontSize();

			if (m_iconSize.x > 0 && m_iconSize.y > 0)
				return m_iconSize;
			if (m_iconSize.x > 0)
				return { m_iconSize.x, m_iconSize.x };
			if (m_iconSize.y > 0)
				return { m_iconSize.y, m_iconSize.y };
			return { fontEdge, fontEdge };
		}

		Vec2 Button::compute_icon_size(f32 textHeightHint) const
		{
			if (!isIconEnabled())
				return { 0, 0 };

			const Vec2 floor = icon_floor();

			// Target height: max(text height, floor.y). Width derives from
			// the icon's natural aspect ratio if the icon is loaded; otherwise
			// match the floor.
			f32 targetH = std::max(textHeightHint, floor.y);

			if (m_icon.isLoaded())
			{
				// When the icon is animated, the "natural size" is one frame,
				// NOT the whole sprite sheet — the sheet's aspect ratio is
				// meaningless (it depends on column/row count). Same logic
				// as ImageLabel::compute_draw_geometry.
				Vec2 nat;
				if (m_animated)
				{
					const auto frame = m_anim.getFrameRect();
					nat = { frame.w, frame.h };
				}
				else
				{
					nat = m_icon.getSize();
				}

				if (nat.x > 0 && nat.y > 0)
				{
					const f32 aspect = nat.x / nat.y;
					return { targetH * aspect, targetH };
				}
			}
			// No natural size known — fall back to whichever is wider.
			return { std::max(floor.x, targetH), targetH };
		}

		// ===================================================================
		// Layout recompute
		//
		// Branches by effective mode:
		//
		//   AutoSize:
		//     1. Measure text at native font size.
		//     2. Compute icon size: height = max(textH, floorH), aspect preserved.
		//     3. Set widget size to fit padding + icon + spacing + text.
		//     4. Place icon and text inside the content area.
		//
		//   Manual / LayoutManaged:
		//     1. Widget size is whatever it currently is (user / layout decided).
		//     2. Measure text at native font size.
		//     3. Icon size = m_iconSize hint (or font-size square if unset),
		//        aspect preserved against icon natural size.
		//     4. Center the text+icon group inside the content area. Overflow
		//        is allowed and clipped at widget level (positions can go
		//        slightly outside if the user picked sizes that don't fit).
		// ===================================================================
		void Button::recompute_layout(void)
		{
			m_layoutDirty = false;

			auto& gfx = getWindow().getGFX();
			const Rectangle pad = getPadding();
			const f32 padW = pad.left() + pad.right();
			const f32 padH = pad.top() + pad.bottom();

			// 1) Text dimensions (native font size in all modes).
			const bool hasText = !isIconOnlyEnabled() && m_text.len() > 0;
			const Vec2 textSize = hasText
				? gfx.getStringDimensions(m_text, getFontSize())
				: Vec2 { 0, 0 };

			// 2) Icon dimensions.
			const eSizeMode mode = effectiveSizeMode();
			Vec2 iconSize { 0, 0 };
			if (isIconEnabled())
			{
				if (mode == eSizeMode::AutoSize)
				{
					// Floor from m_iconSize hint (or font size); rise to text height.
					iconSize = compute_icon_size(textSize.y);
				}
				else
				{
					// Manual / LayoutManaged: just use the configured hint, no
					// floor-against-text logic. If unset, use font size square.
					iconSize = compute_icon_size(0.0f);

					// Cap the icon to the content area so padding is always
					// respected. Aspect ratio is preserved during the cap.
					// AutoSize doesn't need this because it grows the widget
					// to accommodate the icon + padding.
					const f32 availW = std::max(0.0f, getw() - padW);
					const f32 availH = std::max(0.0f, geth() - padH);
					if (iconSize.x > 0 && iconSize.y > 0
						&& (iconSize.x > availW || iconSize.y > availH))
					{
						const f32 sx = (iconSize.x > 0) ? availW / iconSize.x : 1.0f;
						const f32 sy = (iconSize.y > 0) ? availH / iconSize.y : 1.0f;
						const f32 s  = std::min(sx, sy);
						iconSize = { iconSize.x * s, iconSize.y * s };
					}
				}
			}
			const bool hasIcon = iconSize.x > 0 && iconSize.y > 0;

			// 3) Native content extents (icon + spacing + text), no padding yet.
			f32 contentNativeW = 0;
			f32 contentNativeH = 0;
			if (hasIcon)
			{
				contentNativeW += iconSize.x;
				contentNativeH = std::max(contentNativeH, iconSize.y);
			}
			if (hasText)
			{
				if (hasIcon) contentNativeW += m_iconSpacing;
				contentNativeW += textSize.x;
				contentNativeH = std::max(contentNativeH, textSize.y);
			}

			// 4) Decide widget size (AutoSize only) or honor the existing one.
			if (mode == eSizeMode::AutoSize)
			{
				const f32 fallback = (f32)getFontSize();
				const f32 wantW = (contentNativeW > 0 ? contentNativeW : fallback) + padW;
				const f32 wantH = (contentNativeH > 0 ? contentNativeH : fallback) + padH;

				// Only call setSize if the size actually changed — avoids the
				// hover-flicker cycle (theme reload -> applyTheme ->
				// invalidate_layout -> recompute -> setSize -> hover boundary
				// crosses -> ... ).
				if (std::abs(getw() - wantW) > 0.5f || std::abs(geth() - wantH) > 0.5f)
					setSize({ wantW, wantH });
			}
			// else: Manual & LayoutManaged keep current widget size.

			// 5) Place icon and text inside the content area, centered.
			const Vec2 contentSize = getContentBounds().getSize();
			const f32  spacing     = (hasIcon && hasText) ? m_iconSpacing : 0.0f;
			const f32  totalW      = (hasIcon ? iconSize.x : 0) + spacing + (hasText ? textSize.x : 0);
			const f32  startX      = (contentSize.x - totalW) * 0.5f;

			m_drawIconSize = iconSize;
			if (hasIcon)
				m_drawIconOffset = { startX, (contentSize.y - iconSize.y) * 0.5f };
			else
				m_drawIconOffset = { 0, 0 };

			if (hasText)
				m_drawTextOffset = {
					startX + (hasIcon ? iconSize.x + spacing : 0.0f),
					(contentSize.y - textSize.y) * 0.5f
				};
			else
				m_drawTextOffset = { 0, 0 };
		}
	}
}
