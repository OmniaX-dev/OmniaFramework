#ifndef __RICH_STRING_HPP__
#define __RICH_STRING_HPP__

#include <ostd/Color.hpp>

namespace ostd
{
	struct tRichChar
	{
		unsigned char ascii { 0 };
		Color foreground { 0, 0 };
		Color background { 0, 0 };
	};

	class RichString
	{
		public:
			inline RichString(void) {  }
			inline String getText(void) const { return m_text; }
			tRichChar at(uint32_t index) const;
			void add(tRichChar rchar);
			void add(String str, Color fg, Color bg);
			void add(String str);
			void clear(void);
 
		private:
			String m_text { "" };
			std::vector<Color> m_foreground;
			std::vector<Color> m_background; 
	};
}

#endif
