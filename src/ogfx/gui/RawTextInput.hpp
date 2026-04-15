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

#include <ogfx/render/BasicRenderer.hpp>
#include <ostd/utils/Time.hpp>

namespace ogfx
{
	namespace gui
	{
		class RawTextInput : public ostd::Rectangle
		{
			public: class EventListener : public ostd::BaseObject
			{
				public: enum class eEventType { None = 0, TextEntered, KeyPressed, KeyReleased };
				public:
					EventListener(RawTextInput& _parent);
					virtual void handleSignal(ostd::Signal& signal) override;
					inline virtual void onSignalHandled(ostd::Signal& signal) {  }
					inline RawTextInput& getParent(void) { return parent; }

				private:
					RawTextInput& parent;
					eEventType m_lastEvent { eEventType::None };
			};
			public: class CharacterFilter
			{
				public:
					inline virtual bool isValidChar(char c) { return true; }
			};
			public: class Theme
			{
				public:
					ostd::Color textColor { 0, 0, 0, 0 };
					ostd::Color borderColor { 0, 0, 0, 0 };
					ostd::Color backgroundColor { 0, 0, 0, 0 };
					ostd::Color cursorColor { 0, 0, 0, 0 };

					u8 cursorWidth { 0 };
					u8 extraPaddingTop { 0 };
					u8 extraPaddingLeft { 0 };

					bool cursorBlink { true };
					ostd::Counter cursorBlinkCounter { 30 };
			};
			public: struct tDefaultThemes
			{
				inline static const Theme DebugTheme {
					{ 220, 	220, 	255 },	//Text Color
					{ 10, 	20, 	120 },	//Border Color
					{ 0, 	0, 		22 },	//Background Color
					{ 220, 	10, 	10 },	//Cursor Color

					2,						//Cursor Width
					0,						//Extra Padding Top
					0,						//Extra Padding Left

					true,					//Cursor Blink
					{ 30 }				//Cursor Blink Timer
				};
				inline static const Theme DefaultTheme {
					{ 120, 	120, 	180 },	//Text Color
					{ 10, 	20, 	120 },	//Border Color
					{ 0, 	2, 		10 },	//Background Color
					{ 20, 	100, 	255 },	//Cursor Color

					2,						//Cursor Width
					0,						//Extra Padding Top
					0,						//Extra Padding Left

					true,					//Cursor Blink
					{ 30 }				//Cursor Blink Timer
				};
			};
			public: enum eActionEventType { None = 0, Enter, Tab };
			public: class ActionEventData : public ostd::BaseObject
			{
				public:
					inline ActionEventData(RawTextInput& _sender, const String& _senderName, eActionEventType _eventType, ostd::BaseObject& _userData) :
																																								sender(_sender),
																																								senderName(_senderName),
																																								eventType(_eventType),
																																								userData(_userData)
					{
						setTypeName("ogfx::gui::RawTextInput::ActionEventData");
						validate();
					}

				public:
					RawTextInput& sender;
					String senderName { "" };
					eActionEventType eventType { eActionEventType::None };
					ostd::BaseObject& userData { ostd::BaseObject::InvalidRef() };
			};

			public:
				inline RawTextInput(void) { create({ 0.0f, 0.0f }, { 200.0f, 30.0f }, "UnnamedRawTextInput"); }
				inline RawTextInput(const ostd::Vec2& position, const ostd::Vec2& size, const String& name) { create(position, size, name); }
				RawTextInput& create(const ostd::Vec2& position, const ostd::Vec2& size, const String& name);

				virtual void render(ogfx::BasicRenderer2D& gfx);
				virtual void update(void);
				virtual void fixedUpdate(void);

				virtual inline void onRender(ogfx::BasicRenderer2D& gfx) {  }
				virtual inline void onUpdate(void) {  }
				virtual inline void onFixedUpdate(void) {  }

				void setText(const String& text);
				void appendText(const String& text);
				void setCursorPosition(u16 cursorPos);
				void setTheme(Theme theme);

				inline void setEventListener(EventListener& evtl) { m_eventListener = &evtl; }
				inline void setCharacterFilter(CharacterFilter& chrf) { m_charFilter = &chrf; }
				inline void setName(const String& name) { m_name = name; }

				inline EventListener* getEventListener(void) const { return m_eventListener; }
				inline CharacterFilter* getCharacterFilter(void) const { return m_charFilter; }
				inline String getText(void) const { return m_text; }
				inline u16 getCursorPosition(void) const { return m_cursorPosition; }
				inline Theme& getTheme(void) { return m_theme; }
				inline String getName(void) const { return m_name; }
				inline bool isMouseInside(void) const { return m_mouseInside; }

			private:
				EventListener* m_eventListener { nullptr };
				CharacterFilter* m_charFilter { nullptr };
				ogfx::BasicRenderer2D* m_gfx { nullptr };
				i32 m_fontSize { 20 };
				f32 m_paddingX { 0 };

			protected:
				String m_name { "" };

				String m_text { "" };
				u16 m_cursorPosition { 0 };
				bool m_cursorState { true };

				ostd::Counter m_keyRepeatCounter { 1 };
				char m_lastChar { 0 };
				i32 m_lastKeyCode { 0 };

				Theme m_theme;

				bool m_mouseInside { false };

			public:
				inline static const u32 actionEventSignalID { ostd::SignalHandler::newCustomSignal(11400) };
		};
		class RawTextInputEventListener : public RawTextInput::EventListener
		{
			public:
				inline RawTextInputEventListener(ogfx::gui::RawTextInput& _parent, ogfx::WindowCore& _window) : ogfx::gui::RawTextInput::EventListener::EventListener(_parent), window(_window) {  }
				void onSignalHandled(ostd::Signal& signal) override;

			public:
				ogfx::WindowCore& window;
				bool m_ibeamCursorSet { false };
		};
		class RawTextInputNumberCharacterFilter : public RawTextInput::CharacterFilter
		{
			public:
				inline bool isValidChar(char c) override { return c >= '0' && c  <= '9'; }
		};
	}
}
