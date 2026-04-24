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
				setText(text);
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Button");
				disableChildren();
				enableBackground(false);
				setStylesheetCategoryName("button");
				validate();
				return *this;
			}

			void Button::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_textChanged)
					__update_size();
				if (isIconEnabled())
				{

					if (isAnimatedEnabled())
						gfx.drawAnimation(m_anim, getGlobalContentPosition(), m_realIconSize);
					else
						gfx.drawImage(m_icon, getGlobalContentPosition(), m_realIconSize);
				}
				gfx.drawString(getText(), getGlobalContentPosition() + Vec2 { m_realIconSize.x + m_iconSpacing, 0 }, getTextColor(), getFontSize());
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
				__update_size();
			}

			void Button::setIcon(const String& filePath)
			{
				disableIcon();
				if (filePath == "") return;
				if (!ostd::FileSystem::fileExists(filePath))
					return;
				m_icon.destroy();
				m_icon.loadFromFile(filePath, getWindow().getGFX());
				enableIcon();
			}

			void Button::applyTheme(const ostd::Stylesheet& theme)
			{
				enableIcon(getThemeValue<bool>(theme, "showIcon", m_showIcon));
				enableAnimated(getThemeValue<bool>(theme, "icon.animated", m_animated));
				setAnimationData(getThemeValue<AnimationData>(theme, "icon.animation", m_animData));
				String filePath = getThemeValue<String>(theme, "icon.path", m_icon.getFilePath());
				if (filePath != m_icon.getFilePath())
					setIcon(filePath);
				setIconSize(getThemeValue<Vec2>(theme, "icon.size", getIconSize()));
				if (isAnimatedEnabled())
				{
					m_anim.create(m_animData);
					m_anim.setSpriteSheet(m_icon);
				}
				__update_size();
			}

			void Button::__update_size(void)
			{
				m_realIconSize = getIconSize().propy(getGlobalContentBounds().getSize().y);
				auto size = getWindow().getGFX().getStringDimensions(getText(), getFontSize());
				size.x += getPadding().left();
				size.x += getPadding().right();
				size.y += getPadding().top();
				size.y += getPadding().bottom();
				if (isIconEnabled())
					size.x += m_iconSpacing + m_realIconSize.x;
				setSize({ cast<f32>(size.x), cast<f32>(size.y) });
				m_textChanged = false;
			}
		}
	}
}
