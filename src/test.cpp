// #include <ostd/String.hpp>
// #include <ostd/IOHandlers.hpp>
// #include <ostd/Logger.hpp>
// #include <ostd/Console.hpp>

#include <ogfx/WindowBase.hpp>
#include <ogfx/RawTextInput.hpp>
#include <ostd/Signals.hpp>


ostd::ConsoleOutputHandler out;

class Window : public ogfx::WindowBase
{
	public:
		inline Window(void) : m_sigHandler(m_textInput, *this) {  }
		inline void onInitialize(void) override	
		{ 
			enableSignals();
			connectSignal(ostd::tBuiltinSignals::KeyReleased);
			connectSignal(ogfx::gui::RawTextInput::actionEventSignalID);

			m_gfx.init(*this);
			m_gfx.setFont("res/ttf/Courier Prime.ttf");

			float w = getWindowWidth();
			float h = 40.0f;
			m_textInput.create({ 0.0f, (float)(getWindowHeight() - h) }, { w, h }, "MainInputTXT");
			m_textInput.setEventListener(m_sigHandler);
			// m_textInput.setCharacterFilter(m_numCharFilter);
			m_textInput.getTheme().extraPaddingTop = 3;
	 	}
			
		inline void handleSignal(ostd::tSignal& signal) override
		{
			if (signal.ID == ostd::tBuiltinSignals::KeyReleased)
			{
				auto& evtData = (ogfx::KeyEventData&)signal.userData;
				if (evtData.keyCode == SDLK_ESCAPE)
					close();
			}
			if (signal.ID == ogfx::gui::RawTextInput::actionEventSignalID)
			{
				auto& data = (ogfx::gui::RawTextInput::ActionEventData&)signal.userData;
				if (data.senderName != "MainInputTXT")
					return;
				if (data.eventType == ogfx::gui::RawTextInput::eActionEventType::Enter)
				{
					out.fg(ostd::ConsoleColors::Green).p(data.sender.getText()).reset().nl();
					data.sender.setText("");
				}
				else if (data.eventType == ogfx::gui::RawTextInput::eActionEventType::Tab)
				{
					out.fg(ostd::ConsoleColors::Red).p("TAB").reset().nl();
					data.sender.appendText("TAB");
				}
			}
		}

		inline void onRender(void) override
		{
			m_textInput.render(m_gfx);	
		}

		inline void onFixedUpdate(void) override
		{
			m_textInput.fixedUpdate();	
		}

		inline void onUpdate(void) override
		{
			m_textInput.update	();	
		}
		
	private:
		ogfx::gui::RawTextInput m_textInput;
		ogfx::BasicRenderer2D m_gfx;
		ogfx::gui::RawTextInputEventListener m_sigHandler;
		ogfx::gui::RawTextInputNumberCharacterFilter m_numCharFilter;
};

// void test2(const std::string& str) {}
// void test3(const ostd::String& str) {}
// void test4(const char* str) {}

int main(int argc, char** argv)
{
	out.p(STR_BOOL(ostd::Utils::md5("") == "d41d8cd98f00b204e9800998ecf8427e")).nl();
	out.p(STR_BOOL(ostd::Utils::md5("abc") == "900150983cd24fb0d6963f7d28e17f72")).nl();
	out.p(STR_BOOL(ostd::Utils::md5("message digest") == "f96b697d7cb7938d525a2f31aaf161d0")).nl();
	
	Window window;
	window.initialize(1280, 720, "OmniaFramework - Test Window");
	window.setClearColor({ 0, 2	, 15 });
	
	while (window.isRunning())
	{
		window.update();
	}


	// out.fg(ostd::ConsoleColors::Red).p("Hello World!!").reset().nl();

	// ostd::String str1, str2 = "Hello";
	// bool b = str1 == str2;
	// const ostd::String str3 = "CIAO";
	// b = str2 == str1;
	// b = str3 == "CICCIO";
	// // b = "ciao" == str2;
	// str2 = str3;
	// std::string str = "cc";
	// str1 = str;
	// test2(str1);
	// test3(str);
	// str1 = str2 + str;
	// str1 = str2 + "str";
	// str1 = str2 + str3;
	// str1 = str2 + str1;
	// test4(str1);
	// test4(str3);

	// str1 += "ciao";
	// str1 += str;
	// str1 += str1;
	// str1 += str3;
	// str1 += 'c';

	// OX_FATAL(str2);

	// ostd::RegexRichString rgxrstr("Hello World");
	// rgxrstr.fg("Hello", "Blue");
	// std::cout << rgxrstr << "\n";

	// out.nl().nl();
	// ostd::String test_str = "Hello World, my love";
	// ostd::String test_str_2 = "HEELO";
	// ostd::String test_str_3 = "0123456789";
	// out.p("==========\n");
	// test_str.fixedLength(10, ' ', "..........");
	// test_str_2.fixedLength(10);
	// test_str_3.fixedLength(10);
	// out.p(test_str).p("|").nl();
	// out.p(test_str_2).p("|").nl();
	// out.p(test_str_3).p("|").nl();

	// ostd::KeyboardController keyboard;
	// keyboard.disableCommandBuffer();
	
	// out.p("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
	// out.p("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH");
	// out.p("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	// out.p("                                                ");
	// out.p("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

	// ostd::eKeys k = ostd::eKeys::NoKeyPressed;
	// do
	// {
	// 	k = keyboard.waitForKeyPress();
	// } while (k != ostd::eKeys::Escape);

	return 0;
}