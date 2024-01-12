#include <ostd/String.hpp>
#include <ostd/IOHandlers.hpp>
#include <ostd/Logger.hpp>
#include <ostd/Console.hpp>

ostd::ConsoleOutputHandler out;

void test2(const std::string& str) {}
void test3(const ostd::String& str) {}
void test4(const char* str) {}

int main(int argc, char** argv)
{
	out.fg(ostd::ConsoleColors::Red).p("Hello World!!").reset().nl();

	ostd::String str1, str2 = "Hello";
	bool b = str1 == str2;
	const ostd::String str3 = "CIAO";
	b = str2 == str1;
	b = str3 == "CICCIO";
	// b = "ciao" == str2;
	str2 = str3;
	std::string str = "cc";
	str1 = str;
	test2(str1);
	test3(str);
	str1 = str2 + str;
	str1 = str2 + "str";
	str1 = str2 + str3;
	str1 = str2 + str1;
	test4(str1);
	test4(str3);

	str1 += "ciao";
	str1 += str;
	str1 += str1;
	str1 += str3;
	str1 += 'c';

	OX_FATAL(str2);

	ostd::RegexRichString rgxrstr("Hello World");
	rgxrstr.fg("Hello", "Blue");
	std::cout << rgxrstr << "\n";

	out.nl().nl();
	ostd::String test_str = "Hello World, my love";
	ostd::String test_str_2 = "HEELO";
	ostd::String test_str_3 = "0123456789";
	out.p("==========\n");
	test_str.fixedLength(10, ' ', "..........");
	test_str_2.fixedLength(10);
	test_str_3.fixedLength(10);
	out.p(test_str).p("|").nl();
	out.p(test_str_2).p("|").nl();
	out.p(test_str_3).p("|").nl();

	ostd::KeyboardController keyboard;
	keyboard.disableCommandBuffer();
	
	out.p("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
	out.p("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH");
	out.p("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	out.p("                                                ");
	out.p("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

	ostd::eKeys k = ostd::eKeys::NoKeyPressed;
	do
	{
		k = keyboard.waitForKeyPress();
	} while (k != ostd::eKeys::Escape);

	return 0;
}