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

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			Button& Button::create(const String& text)
			{
				m_text = text;
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Button");
				disableChildren();
				enableBackground(false);
				setStylesheetCategoryName("button");
				validate();
				invalidate_layout();
				return *this;
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
						gfx.drawAnimation(m_anim, iconPos, m_drawIconSize);
					else
						gfx.drawImage(m_icon, iconPos, m_drawIconSize);
				}

				// --- Text ---
				if (!isIconOnlyEnabled() && m_text.len() > 0)
				{
					gfx.drawString(
						m_text,
						contentTL + m_drawTextOffset,
						getTextColor(),
						getFontSize(),
						m_contentScale
					);
				}
			}

			void Button::onUpdate(void)
			{
				if (isAnimatedEnabled())
					m_anim.update();
			}

			void Button::onMouseReleased(const Event& event)
			{
				if (!isMouseInside())
					return;
				if (event.mouse->button == MouseEventData::eButton::Left && callback_onActionPerformed)
					callback_onActionPerformed(event);
			}

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
				enableAutoSize(getThemeValue<bool>(theme, "autoSize", m_autoSize));
				enableAnimated(getThemeValue<bool>(theme, "icon.animated", m_animated));
				setAnimationData(getThemeValue<AnimationData>(theme, "icon.animation", m_animData));
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

			Vec2 Button::native_icon_size(void) const
			{
				if (!isIconEnabled())
					return { 0, 0 };

				// Treat font size as the natural icon edge if the user didn't pick one.
				const f32 fontEdge = cast<f32>(getFontSize());

				Vec2 hint = m_iconSize;
				if (hint.x <= 0 && hint.y <= 0)
				{
					if (m_icon.isLoaded())
					{
						// Fit the icon's natural aspect ratio into a font-sized square.
						Vec2 nat = m_icon.getSize();
						if (nat.x > 0 && nat.y > 0)
						{
							if (nat.x >= nat.y)
								return nat.propx(fontEdge);
							return nat.propy(fontEdge);
						}
					}
					return { fontEdge, fontEdge };
				}

				// One axis specified: derive the other from the icon's aspect (if any),
				// otherwise make it square.
				if (hint.x <= 0)
				{
					if (m_icon.isLoaded())
					{
						Vec2 nat = m_icon.getSize();
						if (nat.x > 0 && nat.y > 0)
							return nat.propy(hint.y);
					}
					return { hint.y, hint.y };
				}
				if (hint.y <= 0)
				{
					if (m_icon.isLoaded())
					{
						Vec2 nat = m_icon.getSize();
						if (nat.x > 0 && nat.y > 0)
							return nat.propx(hint.x);
					}
					return { hint.x, hint.x };
				}
				return hint;
			}

			void Button::recompute_layout(void)
			{
				m_layoutDirty = false;

				auto& gfx = getWindow().getGFX();
				const Rectangle pad = getPadding();
				const f32 padW = pad.left() + pad.right();
				const f32 padH = pad.top() + pad.bottom();

				// 1) Native (un-scaled) sizes of the pieces.
				const Vec2 iconNative = native_icon_size();
				const bool hasIcon    = isIconEnabled() && (iconNative.x > 0 && iconNative.y > 0);
				const bool hasText    = !isIconOnlyEnabled() && m_text.len() > 0;
				const Vec2 textNative = hasText
					? gfx.getStringDimensions(m_text, getFontSize())
					: Vec2 { 0, 0 };

				// 2) Native content extents (icon + spacing + text), no padding yet.
				f32 nativeW = 0;
				f32 nativeH = 0;
				if (hasIcon)
				{
					nativeW += iconNative.x;
					nativeH = std::max(nativeH, iconNative.y);
				}
				if (hasText)
				{
					if (hasIcon) nativeW += m_iconSpacing;
					nativeW += textNative.x;
					nativeH = std::max(nativeH, textNative.y);
				}
				if (nativeW <= 0) nativeW = cast<f32>(getFontSize()); // degenerate fallback
				if (nativeH <= 0) nativeH = cast<f32>(getFontSize());

				// 3) Decide the widget size and the content scale.
				if (isAutoSizeEnabled())
				{
					m_contentScale = 1.0f;
					setSize({ nativeW + padW, nativeH + padH });
				}
				else
				{
					// Honor the size the user already set on the widget.
					const f32 availW = std::max(0.0f, getw() - padW);
					const f32 availH = std::max(0.0f, geth() - padH);
					if (availW <= 0 || availH <= 0)
						m_contentScale = 0.0f;
					else
					{
						const f32 sx = availW / nativeW;
						const f32 sy = availH / nativeH;
						m_contentScale = std::min(sx, sy);
					}
				}

				// 4) Compute scaled draw sizes and centered offsets within the content area.
				const Vec2 contentSize = getContentBounds().getSize();
				const Vec2 iconScaled  = hasIcon ? iconNative * m_contentScale : Vec2 { 0, 0 };
				const Vec2 textScaled  = hasText ? textNative * m_contentScale : Vec2 { 0, 0 };
				const f32 spacing      = (hasIcon && hasText) ? (m_iconSpacing * m_contentScale) : 0.0f;

				const f32 totalW = iconScaled.x + spacing + textScaled.x;
				const f32 startX = std::max(0.0f, (contentSize.x - totalW) * 0.5f);

				m_drawIconSize = iconScaled;
				if (hasIcon)
					m_drawIconOffset = { startX, std::max(0.0f, (contentSize.y - iconScaled.y) * 0.5f) };
				else
					m_drawIconOffset = { 0, 0 };

				if (hasText)
					m_drawTextOffset = {
						startX + iconScaled.x + spacing,
						std::max(0.0f, (contentSize.y - textScaled.y) * 0.5f)
					};
				else
					m_drawTextOffset = { 0, 0 };
			}
		}
	}
}
