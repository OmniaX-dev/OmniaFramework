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

#include "RawTextInput.hpp"
#include "../gui/Window.hpp"
#include "../../io/Logger.hpp"
#include "utils/Keycodes.hpp"

namespace ogfx
{
	namespace gui
	{
		RawTextInput::EventListener::EventListener(RawTextInput& _parent) : parent(_parent)
		{
			connectSignal(ostd::BuiltinSignals::KeyPressed);
			connectSignal(ostd::BuiltinSignals::KeyReleased);
			connectSignal(ostd::BuiltinSignals::TextEntered);
			connectSignal(ostd::BuiltinSignals::MouseDragged);
			connectSignal(ostd::BuiltinSignals::MouseMoved);
			connectSignal(ostd::BuiltinSignals::MousePressed);
			connectSignal(ostd::BuiltinSignals::MouseReleased);
			connectSignal(ostd::BuiltinSignals::OnGuiEvent);
			connectSignal(ostd::BuiltinSignals::WindowResized);
		}

		void RawTextInput::EventListener::handleSignal(ostd::Signal& signal)
		{
			if (signal.ID == ostd::BuiltinSignals::TextEntered)
			{
				if (m_lastEvent != eEventType::TextEntered)
				{
					m_lastEvent = eEventType::TextEntered;
					// parent.m_keyRepeatCounter.start();
				}
				auto& data = (ogfx::KeyEventData&)signal.userData;
				if (parent.m_charFilter == nullptr)
				{
					OX_ERROR("Invalid character filter in RawTextInput event listener.");
					return; //TODO: Better error
				}
				if (parent.m_charFilter->isValidChar(data.text))
				{
					if (parent.m_lastChar == data.text && parent.m_keyRepeatCounter.isCounting())
					{
						onSignalHandled(signal);
						return;
					}
					String s1 = parent.m_text.new_substr(0, parent.m_cursorPosition);
					String s2 = "";
					if (parent.m_cursorPosition < parent.m_text.len())
						s2 = parent.m_text.new_substr(parent.m_cursorPosition);
					s1.addChar(data.text).add(s2);
					parent.m_text = s1;
					parent.m_cursorPosition++;
					parent.m_lastChar = data.text;
					parent.m_keyRepeatCounter.start();
					if (parent.m_theme.cursorBlink)
						parent.m_theme.cursorBlinkCounter.start();
					parent.m_cursorState = true;
				}
			}
			else if (signal.ID == ostd::BuiltinSignals::KeyPressed)
			{
				if (m_lastEvent != eEventType::KeyPressed)
				{
					m_lastEvent = eEventType::KeyPressed;
					// parent.m_keyRepeatCounter.start();
				}
				auto& data = (ogfx::KeyEventData&)signal.userData;
				if (parent.m_lastKeyCode == data.keyCode && parent.m_keyRepeatCounter.isCounting())
				{
					onSignalHandled(signal);
					return;
				}
				if (data.keyCode == KeyCode::Backspace)
				{
					parent.m_keyRepeatCounter.start();
					if (parent.m_theme.cursorBlink)
						parent.m_theme.cursorBlinkCounter.start();
					parent.m_cursorState = true;
					parent.m_lastKeyCode = data.keyCode;
					if (parent.m_cursorPosition == 0 || parent.m_text == "")
					{
						onSignalHandled(signal);
						return;
					}
					String s1 = parent.m_text.new_substr(0, parent.m_cursorPosition - 1);
					String s2 = "";
					if (parent.m_cursorPosition < parent.m_text.len())
						s2 = parent.m_text.new_substr(parent.m_cursorPosition);
					parent.m_text = s1 + s2;
					parent.m_cursorPosition--;
				}
				else if (data.keyCode == KeyCode::Left)
				{
					parent.m_keyRepeatCounter.start();
					if (parent.m_theme.cursorBlink)
						parent.m_theme.cursorBlinkCounter.start();
					parent.m_cursorState = true;
					parent.m_lastKeyCode = data.keyCode;
					if (parent.m_cursorPosition > 0)
						parent.m_cursorPosition--;
				}
				else if (data.keyCode == KeyCode::Right)
				{
					parent.m_keyRepeatCounter.start();
					if (parent.m_theme.cursorBlink)
						parent.m_theme.cursorBlinkCounter.start();
					parent.m_cursorState = true;
					parent.m_lastKeyCode = data.keyCode;
					if (parent.m_cursorPosition < parent.m_text.len())
						parent.m_cursorPosition++;
				}
				else if (data.keyCode == KeyCode::Return)
				{
					parent.m_keyRepeatCounter.start();
					parent.m_lastKeyCode = data.keyCode;
					ActionEventData aed(parent, parent.getName(), eActionEventType::Enter, ostd::BaseObject::InvalidRef());
					ostd::SignalHandler::emitSignal(RawTextInput::actionEventSignalID, ostd::Signal::Priority::RealTime, aed);
				}
				else if (data.keyCode == KeyCode::Tab)
				{
					parent.m_keyRepeatCounter.start();
					parent.m_lastKeyCode = data.keyCode;
					ActionEventData aed(parent, parent.getName(), eActionEventType::Tab, ostd::BaseObject::InvalidRef());
					ostd::SignalHandler::emitSignal(RawTextInput::actionEventSignalID, ostd::Signal::Priority::RealTime, aed);
				}
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseMoved)
			{
				auto& data = (ogfx::MouseEventData&)signal.userData;
				if (parent.contains((f32)data.position_x, (f32)data.position_y))
					parent.m_mouseInside = true;
				else
					parent.m_mouseInside = false;
			}
			else if (signal.ID == ostd::BuiltinSignals::MousePressed)
			{
				auto& data = (ogfx::MouseEventData&)signal.userData;
				if (data.button == ogfx::MouseEventData::eButton::Left && parent.m_gfx != nullptr && parent.m_mouseInside)
				{
					String text = parent.m_text;
					Vec2 relativePosition = { (f32)data.position_x, (f32)data.position_y };
					relativePosition -= (parent.getPosition() + parent.m_paddingX + parent.m_theme.extraPaddingLeft);
					if (text.len() > 0)
					{
						String tmpStr1 = "";
						String tmpStr2 = "";
						bool found = false;
						for (i32 i = 0; i < text.len(); i++)
						{
							tmpStr2 = "";
							char c = text[i];
							tmpStr1 += c;
							i32 strWidth1 = parent.m_gfx->getStringDimensions(tmpStr1, parent.m_fontSize).x;
							if (relativePosition.x > strWidth1)
								continue;
							found = true;
							tmpStr2 = tmpStr1.new_substr(0, tmpStr1.len() - 1);
							i32 strWidth2 = (tmpStr2.len() > 0 ? parent.m_gfx->getStringDimensions(tmpStr2, parent.m_fontSize).x : 0);
							i32 d1 = (i32)std::abs(relativePosition.x - strWidth2);
							i32 d2 = (i32)std::abs(strWidth1 - relativePosition.x);
							if (d1 > d2)
								parent.m_cursorPosition = tmpStr1.len();
							else
								parent.m_cursorPosition = tmpStr2.len();
							break;
						}
						if (!found)
							parent.m_cursorPosition = text.len();
					}
					parent.m_cursorState = true;
				}

			}
			onSignalHandled(signal);
		}




		RawTextInput& RawTextInput::create(const Vec2& position, const Vec2& size, const String& name)
		{
			setPosition(position);
			setSize(size);
			m_name = name;
			m_eventListener = new EventListener(*this); //TODO: Delete -- Memory Leak
			m_charFilter = new CharacterFilter(); //TODO: Delete -- Memory Leak
			m_theme = tDefaultThemes::DefaultTheme;
			if (m_theme.cursorBlink)
				m_theme.cursorBlinkCounter.start();
			return *this;
		}

		void RawTextInput::render(ogfx::BasicRenderer2D& gfx)
		{
			m_gfx = &gfx;
			f64 text_size_scale = 0.66666666666666;
			m_paddingX = (4.0f * geth()) / 30.0f;
			m_fontSize = (i32)(geth() * 0.66);
			f32 cursor_height_scale = 0.75f;

			IPoint strSize { 0, 0 };
			if (m_cursorPosition > 0 && m_text != "")
			{
				String s1 = m_text.new_substr(0, m_cursorPosition);
				strSize = gfx.getStringDimensions(s1, m_fontSize);
			}

			gfx.outlinedRect(*this, m_theme.backgroundColor, m_theme.borderColor, 2);
			if (m_text.len() > 0)
				gfx.drawString(m_text, getPosition() + Vec2 { m_paddingX + m_theme.extraPaddingLeft, m_paddingX + m_theme.extraPaddingTop}, m_theme.textColor, m_fontSize);

			if (m_cursorState || !m_theme.cursorBlink)
				gfx.fillRect({ getx() + m_paddingX + m_theme.extraPaddingLeft + strSize.x - 1, gety() + m_paddingX + m_theme.extraPaddingTop, (f32)m_theme.cursorWidth, (f32)m_fontSize * cursor_height_scale }, m_theme.cursorColor);

			onRender(gfx);
		}

		void RawTextInput::update(void)
		{
			m_keyRepeatCounter.update();
			onUpdate();
		}

		void RawTextInput::fixedUpdate(void)
		{
			if (m_theme.cursorBlink)
			{
				m_theme.cursorBlinkCounter.update();
				if (m_theme.cursorBlinkCounter.isDone())
				{
					m_cursorState = !m_cursorState;
					m_theme.cursorBlinkCounter.start();
				}
			}
			onFixedUpdate();
		}

		void RawTextInput::setText(const String& text)
		{
			m_text = text;
			m_cursorPosition = m_text.len();
		}

		void RawTextInput::appendText(const String& text)
		{
			m_text.add(text);
			m_cursorPosition = m_text.len();
		}

		void RawTextInput::setCursorPosition(u16 cursorPos)
		{
			if (cursorPos > m_text.len())
				cursorPos = m_text.len();
			m_cursorPosition = m_text.len();
		}

		void RawTextInput::setTheme(Theme theme)
		{
			m_theme = theme;
			if (m_theme.cursorBlink)
				m_theme.cursorBlinkCounter.start();
			else
				m_theme.cursorBlinkCounter.stop();
		}




		void RawTextInputEventListener::onSignalHandled(ostd::Signal& signal)
		{
			if (signal.ID == ostd::BuiltinSignals::MouseMoved)
			{
				auto& data = (ogfx::MouseEventData&)signal.userData;
				if (getParent().isMouseInside())
				{
					if (!m_ibeamCursorSet)
						window.setCursor(ogfx::WindowCore::eCursor::Text);
					m_ibeamCursorSet = true;
				}
				else
				{
					if (m_ibeamCursorSet)
						window.setCursor(ogfx::WindowCore::eCursor::Default);
					m_ibeamCursorSet = false;
				}
			}
		}
	}
}
