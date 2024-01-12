#include "Console.hpp"

#include <unistd.h>
    
namespace ostd
{
	KeyboardController::KeyboardController(void)
	{
#ifndef WINDOWS_OS
		tcgetattr(0, &initial_settings);
		new_settings = initial_settings;
		new_settings.c_lflag &= ~ICANON;
		new_settings.c_lflag &= ~ECHO;
		new_settings.c_lflag &= ~ISIG;
		new_settings.c_cc[VMIN] = 1;
		new_settings.c_cc[VTIME] = 0;
		tcsetattr(0, TCSANOW, &new_settings);
		peek_character = -1;
#endif
		
		m_cmd = "";
		m_output_enabled = true;
		m_cmd_buffer_enabled = true;
	}
		
	KeyboardController::~KeyboardController(void)
	{
#ifndef WINDOWS_OS
		tcsetattr(0, TCSANOW, &initial_settings);
#endif
	}

#ifndef WINDOWS_OS			
	int KeyboardController::kbhit(void)
	{
		unsigned char ch;
		int nread;
		if (peek_character != -1) return 1;
		new_settings.c_cc[VMIN] = 0;
		tcsetattr(0, TCSANOW, &new_settings);
		nread = read(0, &ch, 1);
		new_settings.c_cc[VMIN] = 1;
		tcsetattr(0, TCSANOW, &new_settings);

		if (nread == 1)
		{
			peek_character = ch;
			return 1;
		}
		return 0;
	}
		
	int KeyboardController::getch(void)
	{
		char ch;

		if (peek_character != -1)
		{
			ch = peek_character;
			peek_character = -1;
		}
		else
			read(0, &ch, 1);
		return ch;
	}
#endif
	
	String KeyboardController::getKeyBuffer(void)
	{
		std::string __str = "";
		for (auto& key : m_key_buff)
		{
			switch (key)
			{
				case eKeys::Backspace:
				case eKeys::Enter:
				case eKeys::Up:
				case eKeys::Down:
				case eKeys::Left:
				case eKeys::Right:
				case eKeys::Escape:
				case eKeys::NoKeyPressed:
					break;
				case eKeys::Tab:
					__str += '\t';
					break;
				default:
					__str += (char)(key);
					break;
			}
		}
		return String(__str);
	}

	String KeyboardController::flushKeyBuffer(void)
	{
		String __str = getKeyBuffer();
		m_key_buff.clear();
		return __str;
	}
	
	eKeys KeyboardController::waitForKeyPress(void) //TODO: Add sleep to this method
	{
		eKeys key = eKeys::NoKeyPressed;
		while (key == eKeys::NoKeyPressed)
			key = getPressedKey();
		return key;
	}

	eKeys KeyboardController::getPressedKey(void)
	{
		int k1 = 0, k2 = 0, k3 = 0;
		eKeys key;
		if(kbhit())
		{
			k1 = getch();
			
			if (k1 == 127 || k1 == 8)
			{
				key = eKeys::Backspace;
				if (isOutputEnabled())
					std::cout << "\b \b" << std::flush;
				if (m_key_buff.size() > 0 && isCommandBufferEnabled())
					m_key_buff.pop_back();
			}
			else if (k1 == 10 || k1 == 13)
			{
				key = eKeys::Enter;
				if (isOutputEnabled())
					std::cout << "\n" << std::flush;
				if (isCommandBufferEnabled())	
					m_cmd = flushKeyBuffer();
			}
			else if (k1 == 9)
			{
				if (isOutputEnabled())
					std::cout << "\t" << std::flush;
				if (isCommandBufferEnabled())
					m_key_buff.push_back(key);
				key = eKeys::Tab;
			}
			else if (k1 >= ' ' && k1 <= '~')
			{
				key = (eKeys)(k1);
				if (isCommandBufferEnabled())
					m_key_buff.push_back(key);
				if (isOutputEnabled())
					std::cout << (char)(key) << std::flush;
			}
#ifndef WINDOWS_OS
			else if (k1 == 27)
			{
				if (!kbhit())
				{
					key = eKeys::Escape;
					return key;    	
				}
				k2 = getch();
				if (k2 == 91)
				{
					k3 = getch();
					if (k3 == 65) key = eKeys::Up;
					else if (k3 == 66) key = eKeys::Down;
					else if (k3 == 67) key = eKeys::Right;
					else if (k3 == 68) key = eKeys::Left;
				}
			}
#else
			else if (k1 == 0xE0)
			{
				k2 = getch();
				if (k2 == 0x48) key = eKeys::Up;
				else if (k2 == 0x50) key = eKeys::Down;
				else if (k2 == 0x4D) key = eKeys::Right;
				else if (k2 == 0x4B) key = eKeys::Left;
				else key = eKeys::NoKeyPressed;
		}
			else if (k1 == 27)
			{
				key = eKeys::Escape;
			}
#endif
			
		}
		else key = eKeys::NoKeyPressed;
		return key;
	}
}    

