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
			class Label : public Widget
			{
				public:
					inline Label(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
					inline Label(WindowCore& window, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					Label& create(const String& text);
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void setText(const String& text);
					inline String getText(void) const { return m_text; }

				private:
					void __update_size(ogfx::BasicRenderer2D& gfx);

				private:
					String m_text { "" };
					bool m_textChanged { false };
			};
			class ImageLabel : public Widget
			{
				public:
					inline ImageLabel(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
					inline ImageLabel(WindowCore& window, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					ImageLabel& create(const String& filePath);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void onUpdate(void) override;
					void setImage(const String& filePath);
					inline Image& getImage(void) { return m_image; }
					OSTD_BOOL_PARAM_GETSET_E(Animated, m_animated);
					OSTD_PARAM_GETSET(AnimationData, AnimationData, m_animData);

				private:
					Image m_image;
					Animation m_anim;
					AnimationData m_animData;
					bool m_animated { false };
			};
		}
	}
}
