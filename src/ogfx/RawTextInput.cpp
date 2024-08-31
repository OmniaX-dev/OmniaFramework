#include "RawTextInput.hpp"
#include <ogfx/WindowBase.hpp>
#include <ostd/Logger.hpp>

namespace ogfx
{
	namespace gui
	{
		RawTextInput::EventListener::EventListener(RawTextInput& _parent) : parent(_parent)
		{
			connectSignal(ostd::tBuiltinSignals::KeyPressed);
			connectSignal(ostd::tBuiltinSignals::KeyReleased);
			connectSignal(ostd::tBuiltinSignals::TextEntered);
			connectSignal(ostd::tBuiltinSignals::MouseDragged);
			connectSignal(ostd::tBuiltinSignals::MouseMoved);
			connectSignal(ostd::tBuiltinSignals::MousePressed);
			connectSignal(ostd::tBuiltinSignals::MouseReleased);
			connectSignal(ostd::tBuiltinSignals::OnGuiEvent);
			connectSignal(ostd::tBuiltinSignals::WindowResized);
		}

		void RawTextInput::EventListener::handleSignal(ostd::tSignal& signal)
		{
			if (signal.ID == ostd::tBuiltinSignals::TextEntered)
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
					ostd::String s1 = parent.m_text.new_substr(0, parent.m_cursorPosition);
					ostd::String s2 = "";
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
			else if (signal.ID == ostd::tBuiltinSignals::KeyPressed)
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
				if (data.keyCode == SDLK_BACKSPACE)
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
					ostd::String s1 = parent.m_text.new_substr(0, parent.m_cursorPosition - 1);
					ostd::String s2 = "";
					if (parent.m_cursorPosition < parent.m_text.len())
						s2 = parent.m_text.new_substr(parent.m_cursorPosition);
					parent.m_text = s1 + s2;
					parent.m_cursorPosition--;
				}
				else if (data.keyCode == SDLK_LEFT)
				{
					parent.m_keyRepeatCounter.start();
					if (parent.m_theme.cursorBlink)
						parent.m_theme.cursorBlinkCounter.start();
					parent.m_cursorState = true;
					parent.m_lastKeyCode = data.keyCode;
					if (parent.m_cursorPosition > 0)
						parent.m_cursorPosition--;
				}
				else if (data.keyCode == SDLK_RIGHT)
				{
					parent.m_keyRepeatCounter.start();
					if (parent.m_theme.cursorBlink)
						parent.m_theme.cursorBlinkCounter.start();
					parent.m_cursorState = true;
					parent.m_lastKeyCode = data.keyCode;
					if (parent.m_cursorPosition < parent.m_text.len())
						parent.m_cursorPosition++;
				}
				else if (data.keyCode == SDLK_RETURN)
				{
					parent.m_keyRepeatCounter.start();
					parent.m_lastKeyCode = data.keyCode;
					ActionEventData aed(parent, parent.getName(), eActionEventType::Enter, ostd::BaseObject::InvalidRef());
					ostd::SignalHandler::emitSignal(RawTextInput::actionEventSignalID, ostd::tSignalPriority::RealTime, aed);
				}
				else if (data.keyCode == SDLK_TAB)
				{
					parent.m_keyRepeatCounter.start();
					parent.m_lastKeyCode = data.keyCode;
					ActionEventData aed(parent, parent.getName(), eActionEventType::Tab, ostd::BaseObject::InvalidRef());
					ostd::SignalHandler::emitSignal(RawTextInput::actionEventSignalID, ostd::tSignalPriority::RealTime, aed);
				}
			}
			else if (signal.ID == ostd::tBuiltinSignals::MouseMoved)
			{
				auto& data = (ogfx::MouseEventData&)signal.userData;
				if (parent.contains((float)data.position_x, (float)data.position_y))
					parent.m_mouseInside = true;
				else
					parent.m_mouseInside = false;
			}
			else if (signal.ID == ostd::tBuiltinSignals::MousePressed)
			{
				auto& data = (ogfx::MouseEventData&)signal.userData;
				if (data.button == ogfx::MouseEventData::eButton::Left && parent.m_gfx != nullptr && parent.m_mouseInside)
				{
					ostd::String text = parent.m_text;
					ostd::Vec2 relativePosition = { (float)data.position_x, (float)data.position_y };
					relativePosition -= (parent.getPosition() + parent.m_paddingX + parent.m_theme.extraPaddingLeft);
					if (text.len() > 0)
					{
						ostd::String tmpStr1 = "";
						ostd::String tmpStr2 = "";
						bool found = false;
						for (int32_t i = 0; i < text.len(); i++)
						{
							tmpStr2 = "";
							char c = text[i];
							tmpStr1 += c;
							int32_t strWidth1 = parent.m_gfx->getStringSize(tmpStr1, parent.m_fontSize).x;
							if (relativePosition.x > strWidth1)
								continue;
							found = true;
							tmpStr2 = tmpStr1.new_substr(0, tmpStr1.len() - 1);
							int32_t strWidth2 = (tmpStr2.len() > 0 ? parent.m_gfx->getStringSize(tmpStr2, parent.m_fontSize).x : 0);
							int32_t d1 = (int32_t)std::abs(relativePosition.x - strWidth2);
							int32_t d2 = (int32_t)std::abs(strWidth1 - relativePosition.x);
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




		RawTextInput& RawTextInput::create(const ostd::Vec2& position, const ostd::Vec2& size, const ostd::String& name)
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
			double text_size_scale = 0.66666666666666;
			m_paddingX = (4.0f * geth()) / 30.0f;
			m_fontSize = (int32_t)(geth() * 0.66);
			float cursor_height_scale = 0.75f;
			
			ostd::IPoint strSize { 0, 0 };
			if (m_cursorPosition > 0 && m_text != "")
			{
				ostd::String s1 = m_text.new_substr(0, m_cursorPosition);
				strSize = gfx.getStringSize(s1, m_fontSize);
			}

			gfx.outlinedRect(*this, m_theme.backgroundColor, m_theme.borderColor, 2);
			if (m_text.len() > 0)
				gfx.drawString(m_text, getPosition() + ostd::Vec2 { m_paddingX + m_theme.extraPaddingLeft, m_paddingX + m_theme.extraPaddingTop}, m_theme.textColor, m_fontSize);
			
			if (m_cursorState || !m_theme.cursorBlink)
				gfx.fillRect({ getx() + m_paddingX + m_theme.extraPaddingLeft + strSize.x - 1, gety() + m_paddingX + m_theme.extraPaddingTop, (float)m_theme.cursorWidth, (float)m_fontSize * cursor_height_scale }, m_theme.cursorColor);
			  
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

		void RawTextInput::setText(const ostd::String& text)
		{
			m_text = text;
			m_cursorPosition = m_text.len();
		}

		void RawTextInput::appendText(const ostd::String& text)
		{
			m_text.add(text);
			m_cursorPosition = m_text.len();
		}

		void RawTextInput::setCursorPosition(uint16_t cursorPos)
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




		void RawTextInputEventListener::onSignalHandled(ostd::tSignal& signal)
		{
			if (signal.ID == ostd::tBuiltinSignals::MouseMoved)
			{
				auto& data = (ogfx::MouseEventData&)signal.userData;
				if (getParent().isMouseInside())
				{
					if (!m_ibeamCursorSet)
						window.setCursor(ogfx::WindowBase::eCursor::IBeam);
					m_ibeamCursorSet = true;
				}
				else
				{
					if (m_ibeamCursorSet)
						window.setCursor(ogfx::WindowBase::eCursor::Arrow);
					m_ibeamCursorSet = false;
				}
			}
		}
	}
}