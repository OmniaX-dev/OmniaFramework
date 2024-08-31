#pragma once

#include <ogfx/BasicRenderer.hpp>
#include <ostd/Utils.hpp>
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
					virtual void handleSignal(ostd::tSignal& signal) override;
					inline virtual void onSignalHandled(ostd::tSignal& signal) {  }
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

					uint8_t cursorWidth { 0 };
					uint8_t extraPaddingTop { 0 };
					uint8_t extraPaddingLeft { 0 };

					bool cursorBlink { true };
					ostd::Counter cursorBlinkCounter { 1 };
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
					{ 1 }				//Cursor Blink Timer
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
					{ 1 }				//Cursor Blink Timer
				};
			};
			public: enum eActionEventType { None = 0, Enter, Tab };
			public: class ActionEventData : public ostd::BaseObject
			{
				public:
					inline ActionEventData(RawTextInput& _sender, const ostd::String& _senderName, eActionEventType _eventType, ostd::BaseObject& _userData) :
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
					ostd::String senderName { "" };
					eActionEventType eventType { eActionEventType::None };
					ostd::BaseObject& userData { ostd::BaseObject::InvalidRef() };
			};

			public:
				inline RawTextInput(void) { create({ 0.0f, 0.0f }, { 200.0f, 30.0f }, "UnnamedRawTextInput"); }
				inline RawTextInput(const ostd::Vec2& position, const ostd::Vec2& size, const ostd::String& name) { create(position, size, name); }
				RawTextInput& create(const ostd::Vec2& position, const ostd::Vec2& size, const ostd::String& name);

				virtual void render(ogfx::BasicRenderer2D& gfx);
				virtual void update(void);
				virtual void fixedUpdate(void);

				virtual inline void onRender(ogfx::BasicRenderer2D& gfx) {  }
				virtual inline void onUpdate(void) {  }
				virtual inline void onFixedUpdate(void) {  }
				
				void setText(const ostd::String& text);
				void appendText(const ostd::String& text);
				void setCursorPosition(uint16_t cursorPos);
				void setTheme(Theme theme);

				inline void setEventListener(EventListener& evtl) { m_eventListener = &evtl; }
				inline void setCharacterFilter(CharacterFilter& chrf) { m_charFilter = &chrf; }
				inline void setName(const ostd::String& name) { m_name = name; }

				inline EventListener* getEventListener(void) const { return m_eventListener; }
				inline CharacterFilter* getCharacterFilter(void) const { return m_charFilter; }
				inline ostd::String getText(void) const { return m_text; }
				inline uint16_t getCursorPosition(void) const { return m_cursorPosition; }
				inline Theme& getTheme(void) { return m_theme; }
				inline ostd::String getName(void) const { return m_name; }
				inline bool isMouseInside(void) const { return m_mouseInside; }

			private:
				EventListener* m_eventListener { nullptr };
				CharacterFilter* m_charFilter { nullptr };
				ogfx::BasicRenderer2D* m_gfx { nullptr };
				int32_t m_fontSize { 20 };
				float m_paddingX { 0 };

			protected:
				ostd::String m_name { "" };

				ostd::String m_text { "" };
				uint16_t m_cursorPosition { 0 };
				bool m_cursorState { true };

				ostd::Counter m_keyRepeatCounter { 80 };
				char m_lastChar { 0 };
				int32_t m_lastKeyCode { 0 };

				Theme m_theme;
				
				bool m_mouseInside { false };
			
			public:
				inline static const uint32_t actionEventSignalID { ostd::SignalHandler::newCustomSignal(11400) };
		};
		class RawTextInputEventListener : public RawTextInput::EventListener
		{
			public:
				inline RawTextInputEventListener(ogfx::gui::RawTextInput& _parent, ogfx::WindowBase& _window) : ogfx::gui::RawTextInput::EventListener::EventListener(_parent), window(_window) {  }
				void onSignalHandled(ostd::tSignal& signal) override;

			public:
				ogfx::WindowBase& window;
				bool m_ibeamCursorSet { false };
		};
		class RawTextInputNumberCharacterFilter : public RawTextInput::CharacterFilter
		{
			public:
				inline bool isValidChar(char c) override { return c >= '0' && c  <= '9'; }
		};
	}
}