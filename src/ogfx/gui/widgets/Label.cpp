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

#include "Label.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../../../ostd/io/FileSystem.hpp"
#include "../Window.hpp"
#include <algorithm>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		// ===================================================================
		// Label
		// ===================================================================
		Label& Label::create(const String& text)
		{
			m_text = text;
			setPadding({ 5, 5, 5, 5 });
			setTypeName("ogfx::gui::Label");
			disableChildren();
			enableBackground(false);
			setStylesheetCategoryName("label");
			validate();
			invalidate_layout();
			return *this;
		}

		void Label::setSizeMode(eSizeMode mode)
		{
			m_sizeMode = mode;
			m_modeExplicitlySet = true;
			invalidate_layout();
		}

		eSizeMode Label::effectiveSizeMode(void) const
		{
			if (!m_modeExplicitlySet && getParent() && getParent()->hasLayout())
				return eSizeMode::LayoutManaged;
			return m_sizeMode;
		}

		void Label::onBoundsChanged(const Vec2& /*newSize*/)
		{
			invalidate_layout();
		}

		void Label::setText(const String& text)
		{
			m_text = text;
			invalidate_layout();
		}

		void Label::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			if (m_layoutDirty)
				recompute_layout();
			gfx.drawString(
				m_text,
				getGlobalContentPosition() + m_drawTextOffset,
				getTextColor(),
				getFontSize()
			);
		}

		void Label::recompute_layout(void)
		{
			m_layoutDirty = false;

			auto& gfx = getWindow().getGFX();
			const Vec2 textSize = gfx.getStringDimensions(m_text, getFontSize());
			const Rectangle pad = getPadding();
			const eSizeMode mode = effectiveSizeMode();

			if (mode == eSizeMode::AutoSize)
			{
				const f32 wantW = textSize.x + pad.left() + pad.right();
				const f32 wantH = textSize.y + pad.top() + pad.bottom();

				// Avoid feedback loop on hover/focus theme reloads — only resize
				// if the desired size actually changed by a meaningful amount.
				if (std::abs(getw() - wantW) > 0.5f || std::abs(geth() - wantH) > 0.5f)
					setSize({ wantW, wantH });

				// Auto-size means the content fills the content box exactly,
				// so the text offset is (0, 0).
				m_drawTextOffset = { 0, 0 };
			}
			else
			{
				// Manual / LayoutManaged: center text inside the (user/layout-given)
				// content area. Overflow is clipped at the widget edge.
				const Vec2 contentSize = getContentBounds().getSize();
				m_drawTextOffset = {
					(contentSize.x - textSize.x) * 0.5f,
					(contentSize.y - textSize.y) * 0.5f
				};
			}
		}


		// ===================================================================
		// ImageLabel
		// ===================================================================
		ImageLabel& ImageLabel::create(const String& filePath)
		{
			setImage(filePath);
			setPadding({ 0, 0, 0, 0 });
			setTypeName("ogfx::gui::ImageLabel");
			disableChildren();
			disableBackground();
			disableBorder();
			setStylesheetCategoryName("image");
			setSize(64, 64);
			validate();
			return *this;
		}

		void ImageLabel::applyTheme(const ostd::Stylesheet& theme)
		{
			enableAnimated(getThemeValue<bool>(theme, "animated", m_animated));
			setTintColor(getThemeValue<Color>(theme, "tint", getTintColor()));
			setAnimationData(getThemeValue<AnimationData>(theme, "animation", m_animData));
			String filePath = getThemeValue<String>(theme, "path", m_image.getFilePath());
			if (filePath != m_image.getFilePath())
				setImage(filePath);
			// NOTE: the old code set widget size from theme here, which is wrong
			// once layouts come into play. The theme's "size" affects m_imageSize
			// (which is meaningful in Native mode). Widget size stays under
			// user/layout control.
			setImageSize(getThemeValue<Vec2>(theme, "size", getImageSize()));
			if (isAnimatedEnabled())
			{
				m_anim.create(m_animData);
				m_anim.setSpriteSheet(m_image);
			}
		}

		void ImageLabel::setImage(const String& filePath)
		{
			if (filePath == "") return;
			if (!ostd::FileSystem::fileExists(filePath))
				return;
			m_image.destroy();
			m_image.loadFromFile(filePath, getWindow().getGFX());
		}

		void ImageLabel::onUpdate(void)
		{
			if (isAnimatedEnabled())
				m_anim.update();
		}

		void ImageLabel::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			Vec2 drawPos, drawSize;
			compute_draw_geometry(drawPos, drawSize);

			if (drawSize.x <= 0 || drawSize.y <= 0)
				return;

			if (isAnimatedEnabled())
				gfx.drawAnimation(m_anim, drawPos, drawSize, getTintColor());
			else
				gfx.drawImage(m_image, drawPos, drawSize, getTintColor());
		}

		void ImageLabel::compute_draw_geometry(Vec2& outPos, Vec2& outSize) const
		{
			const Vec2 contentTL   = getGlobalContentPosition();
			const Vec2 contentSize = getContentBounds().getSize();

			// Natural size: when the image is a sprite sheet driven by an
			// Animation, one frame is the natural unit, not the whole sheet.
			// (Otherwise letterboxing would respect the sheet's aspect ratio,
			// which is meaningless — it depends on column/row count.)
			Vec2 nat { 0, 0 };
			if (m_image.isLoaded())
			{
				if (m_animated)
				{
					const auto frame = m_anim.getFrameRect();
					nat = { frame.w, frame.h };
				}
				else
				{
					nat = m_image.getSize();
				}
			}

			if (m_fit == eImageFit::Fit)
			{
				if (!m_keepAspect)
				{
					// Stretch to fill, no aspect preservation.
					outPos  = contentTL;
					outSize = contentSize;
					return;
				}

				// Letterbox: scale uniformly to fit, then center.
				if (nat.x <= 0 || nat.y <= 0)
				{
					// No natural size known — fall back to filling.
					outPos  = contentTL;
					outSize = contentSize;
					return;
				}
				const f32 sx = contentSize.x / nat.x;
				const f32 sy = contentSize.y / nat.y;
				const f32 s  = std::min(sx, sy);
				outSize = { nat.x * s, nat.y * s };
				outPos  = contentTL + Vec2 {
					(contentSize.x - outSize.x) * 0.5f,
					(contentSize.y - outSize.y) * 0.5f
				};
				return;
			}

			// Native: draw at m_imageSize, centered. Overflow is clipped by
			// the widget's automatic clipping at the renderer level.
			outSize = m_imageSize;
			outPos  = contentTL + Vec2 {
				(contentSize.x - outSize.x) * 0.5f,
				(contentSize.y - outSize.y) * 0.5f
			};
		}
	}
}
