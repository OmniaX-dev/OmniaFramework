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
					OSTD_BOOL_PARAM_GETSET_E(Animated, m_animated);
					OSTD_BOOL_PARAM_GETSET_E(AutoSize, m_autoSize);
					OSTD_BOOL_PARAM_GETSET_E(Icon, m_showIcon);
					OSTD_PARAM_GETSET(ostd::AnimationData, AnimationData, m_animData);
					OSTD_PARAM_GETSET(Vec2, IconSize, m_iconSize);

				private:
					void __update_size(void);

				private:
					String m_text { "" };
					bool m_textChanged { true };
					Image m_icon;
					Vec2 m_realIconSize { 0, 0 };
					bool m_autoSize { true };

					bool m_showIcon { false };
					ostd::AnimationData m_animData;
					Animation m_anim;
					bool m_animated { false };
					f32 m_iconSpacing { 10 };
					Vec2 m_iconSize { 0, 0 };
			};
		}
	}
}
