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

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			Label& Label::create(const String& text)
			{
				setText(text);
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Label");
				disableChildren();
				enableBackground(false);
				setStylesheetCategoryName("label");
				validate();
				return *this;
			}

			void Label::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_textChanged)
					__update_size(gfx);
				gfx.drawString(getText(), getGlobalContentPosition(), getTextColor(), getFontSize());
			}

			void Label::setText(const String& text)
			{
				m_text = text;
				m_textChanged = true;
			}

			void Label::__update_size(ogfx::BasicRenderer2D& gfx)
			{
				auto size = gfx.getStringDimensions(getText(), getFontSize());
				size.x += getPadding().left();
				size.x += getPadding().right();
				size.y += getPadding().top();
				size.y += getPadding().bottom();
				setSize({ cast<f32>(size.x), cast<f32>(size.y) });
				m_textChanged = false;
			}




			ImageLabel& ImageLabel::create(const String& filePath)
			{
				setImage(filePath);
				setPadding({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::ImageLabel");
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
				setAnimationData(getThemeValue<AnimationData>(theme, "animation", m_animData));
				String filePath = getThemeValue<String>(theme, "path", m_image.getFilePath());
				if (filePath != m_image.getFilePath())
					setImage(filePath);
				setSize(getThemeValue<Vec2>(theme, "size", getSize()));
				if (isAnimatedEnabled())
				{
					m_anim.create(m_animData);
					m_anim.setSpriteSheet(m_image);
				}
			}

			void ImageLabel::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (isAnimatedEnabled())
					gfx.drawAnimation(m_anim, getGlobalPosition() + getPadding().topLeft(), getSize() - getPadding().bottomRight());
				else
					gfx.drawImage(m_image, getGlobalPosition() + getPadding().topLeft(), getSize() - getPadding().bottomRight());
			}

			void ImageLabel::onUpdate(void)
			{
				if (isAnimatedEnabled())
					m_anim.update();
			}

			void ImageLabel::setImage(const String& filePath)
			{
				if (filePath == "") return;
				if (!ostd::FileSystem::fileExists(filePath))
					return;
				m_image.destroy();
				m_image.loadFromFile(filePath, getWindow().getGFX());
			}

		}
	}
}
